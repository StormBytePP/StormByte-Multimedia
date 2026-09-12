/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-Multimedia source in this
 * file. Third-party components — including FFmpeg and embedded trained data —
 * remain under their own licenses and are not covered by the commercial grant.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-Multimedia. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#include <StormByte/multimedia/backend/pipeline/detail/muxer/matroska/attachment.hxx>
#include <StormByte/multimedia/backend/pipeline/detail/muxer/matroska/container.hxx>
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/muxer.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/plan.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/pipeline/track.hxx>
#include <StormByte/multimedia/type.hxx>

#ifdef WINDOWS
#include <StormByte/string.hxx>
#endif

#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <utility>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavcodec/packet.h>
	#include <libavformat/avformat.h>
	#include <libavutil/dict.h>
	#include <libavutil/error.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/mem.h>
	#include <libavutil/rational.h>
}

using StormByte::Multimedia::Type;

namespace {
	const AVRational NanoTimeBase{1, 1000000000};
	constexpr const char WritingApp[] = "StormByte-Multimedia " STORMBYTE_MULTIMEDIA_VERSION;

	std::int64_t NsToTicks(std::int64_t ns, AVRational time_base) noexcept {
		if (time_base.num <= 0 || time_base.den <= 0)
			return AV_NOPTS_VALUE;
		return av_rescale_q(ns, NanoTimeBase, time_base);
	}

	std::string AvError(int err) noexcept {
		char buf[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(err, buf, sizeof(buf));
		return buf;
	}

	std::span<const std::byte> UnreadSpan(const StormByte::Buffer::FIFO& fifo) noexcept {
		const auto& stored = fifo.Data();
		const auto avail = fifo.AvailableBytes();
		if (avail == 0 || avail > stored.size())
			return {};
		return std::span<const std::byte>{stored.data() + (stored.size() - avail), avail};
	}

	AVPacket* MakeAvPacket(StormByte::Multimedia::Pipeline::Packet& packet) noexcept {
		AVPacket* raw = av_packet_alloc();
		if (!raw)
			return nullptr;

		auto& fifo = packet.Payload();
		const auto size = fifo.AvailableBytes();
		if (size > 0) {
			if (av_new_packet(raw, static_cast<int>(size)) < 0) {
				av_packet_free(&raw);
				return nullptr;
			}
			const auto view = UnreadSpan(fifo);
			if (view.size() != size) {
				av_packet_free(&raw);
				return nullptr;
			}
			std::memcpy(raw->data, view.data(), size);
			if (!fifo.Drop(size)) {
				av_packet_free(&raw);
				return nullptr;
			}
		}
		if (packet.KeyFrame())
			raw->flags |= AV_PKT_FLAG_KEY;

		for (const auto& side : packet.Attachments()) {
			const auto view = UnreadSpan(side.Payload());
			if (view.empty())
				continue;
			enum AVPacketSideDataType type = AV_PKT_DATA_NB;
			switch (side.Kind()) {
				case StormByte::Multimedia::Pipeline::SideDataKind::HdrPlus:
					type = AV_PKT_DATA_DYNAMIC_HDR10_PLUS;
					break;
				case StormByte::Multimedia::Pipeline::SideDataKind::MasteringDisplay:
					type = AV_PKT_DATA_MASTERING_DISPLAY_METADATA;
					break;
				case StormByte::Multimedia::Pipeline::SideDataKind::ContentLight:
					type = AV_PKT_DATA_CONTENT_LIGHT_LEVEL;
					break;
				default:
					break;
			}
			if (type == AV_PKT_DATA_NB)
				continue;
			uint8_t* dst = av_packet_new_side_data(raw, type, view.size());
			if (!dst) {
				av_packet_free(&raw);
				return nullptr;
			}
			std::memcpy(dst, view.data(), view.size());
		}
		return raw;
	}

	bool PlanWantsAttachments(const StormByte::Multimedia::Pipeline::Muxer& owner) noexcept {
		const auto& plan = owner.Plan();
		if (!plan)
			return false;
		for (const auto& track : plan->Tracks()) {
			if (track && track->Type() == StormByte::Multimedia::Type::Attachment)
				return true;
		}
		return false;
	}
}

namespace StormByte::Multimedia::Backend::Pipeline::Detail::Muxer::Matroska {
	Container::Container() noexcept
	: m_ctx(nullptr), m_file(nullptr), m_header(false), m_trailer(false) {}

	Container::~Container() noexcept {
		FreeParams();
		Close();
	}

	void Container::FreeParams() noexcept {
		for (auto& [index, track] : m_tracks) {
			if (track.params) {
				avcodec_parameters_free(&track.params);
				track.params = nullptr;
			}
		}
	}

	bool Container::IsOpen() const noexcept {
		return m_ctx != nullptr;
	}

	bool Container::HeaderWritten() const noexcept {
		return m_header;
	}

	int Container::Resolve(int track) const noexcept {
		if (m_tracks.contains(track))
			return track;
		const auto it = m_inToOut.find(track);
		if (it != m_inToOut.end())
			return it->second;
		return -1;
	}

	bool Container::BindPath(StormByte::Multimedia::Pipeline::Muxer& owner,
		const std::filesystem::path& path) noexcept {
		if (m_ctx) {
			owner.Fail("muxer destination is already bound");
			return false;
		}
		if (path.empty()) {
			owner.Fail("mux destination path is empty");
			return false;
		}
		std::string url;
		try {
#ifdef WINDOWS
			url = StormByte::String::UTF8Encode(path.wstring());
#else
			url = path.string();
#endif
		}
		catch (...) {
			owner.Fail("could not convert destination path to UTF-8");
			return false;
		}
		const auto ext = owner.Destination().Extension();
		const std::string dummy = ext.empty() ? url : ("out." + std::string(ext));
		const AVOutputFormat* oformat = av_guess_format(nullptr, dummy.c_str(), nullptr);
		if (!oformat) {
			owner.Fail("could not guess output format from container");
			return false;
		}
		AVFormatContext* ctx = nullptr;
		if (avformat_alloc_output_context2(&ctx, const_cast<AVOutputFormat*>(oformat), nullptr, url.c_str()) < 0 || !ctx) {
			owner.Fail("avformat_alloc_output_context2 failed");
			return false;
		}
		m_ctx = ctx;
		m_path = path;
		if (!(ctx->oformat->flags & AVFMT_NOFILE)) {
			if (avio_open(&ctx->pb, url.c_str(), AVIO_FLAG_WRITE) < 0) {
				owner.Fail("avio_open failed");
				return false;
			}
		}
		return WriteHeaderIfReady(owner);
	}

	bool Container::ReserveEncoder(StormByte::Multimedia::Pipeline::Muxer& owner,
		StormByte::Multimedia::Pipeline::Encoder& encoder) noexcept {
		if (m_header) {
			owner.Fail("cannot add a track after the header");
			return false;
		}
		if (encoder.Index() < 0) {
			owner.Fail("encoder output index is invalid");
			return false;
		}
		if (m_tracks.contains(encoder.Index())) {
			owner.Fail("duplicate mux output index");
			return false;
		}
		Track track;
		track.encoder = &encoder;
		track.language = owner.Language(encoder.Index());
		track.title = owner.Title(encoder.Index());
		m_tracks.emplace(encoder.Index(), std::move(track));
		return true;
	}

	bool Container::ReserveRemux(StormByte::Multimedia::Pipeline::Muxer& owner,
		int inIndex) noexcept {
		if (m_header) {
			owner.Fail("cannot add a track after the header");
			return false;
		}
		if (inIndex < 0) {
			owner.Fail("remux source stream is invalid");
			return false;
		}
		const int out = static_cast<int>(m_tracks.size());
		if (m_tracks.contains(out)) {
			owner.Fail("duplicate mux output index");
			return false;
		}
		Track track;
		track.inIndex = inIndex;
		track.language = owner.Language(out);
		track.title = owner.Title(out);
		m_tracks.emplace(out, std::move(track));
		m_inToOut.emplace(inIndex, out);
		return true;
	}

	bool Container::BindAttachments(StormByte::Multimedia::Pipeline::Muxer& owner,
		const File& file) noexcept {
		if (m_header) {
			owner.Fail("cannot bind attachments after the header");
			return false;
		}
		if (!PlanWantsAttachments(owner)) {
			m_file = nullptr;
			return true;
		}
		m_file = &file;
		if (!owner.Destination().HasAccess(Access{Operation::Attach})) {
			owner.Fail("destination container does not support attachments");
			return false;
		}
		return true;
	}

	bool Container::Push(StormByte::Multimedia::Pipeline::Muxer& owner,
		const std::shared_ptr<StormByte::Multimedia::Pipeline::Packet>& packet) noexcept {
		if (!packet) {
			owner.Fail("empty packet");
			return false;
		}
		if (Resolve(packet->Track()) < 0) {
			owner.Fail("packet stream index is not a mux track");
			return false;
		}
		if (!m_ctx) {
			m_queue.push_back(packet);
			return true;
		}
		if (!WriteHeaderIfReady(owner))
			return false;
		if (m_header) {
			while (!m_queue.empty()) {
				std::shared_ptr<StormByte::Multimedia::Pipeline::Packet> leftover = std::move(m_queue.front());
				m_queue.pop_front();
				if (!leftover || !WritePacket(owner, *leftover))
					return false;
			}
			return WritePacket(owner, *packet);
		}
		m_queue.push_back(packet);
		return true;
	}

	bool Container::WriteHeaderIfReady(StormByte::Multimedia::Pipeline::Muxer& owner) noexcept {
		if (owner.Failed() || m_header)
			return !owner.Failed();
		if (!m_ctx)
			return true;
		if (m_tracks.empty())
			return true;
		for (const auto& [index, track] : m_tracks) {
			if (!track.encoder)
				continue;
			if (track.encoder->Failed()) {
				owner.Fail("encoder failed");
				return false;
			}
			if (!*track.encoder)
				return true;
		}
		for (auto& [index, track] : m_tracks) {
			if (track.encoder || track.params)
				continue;
			if (!owner.RemuxCodec(track.inIndex,
					reinterpret_cast<void*&>(track.params),
					&track.srcTb))
				return true;
		}

		int expected = 0;
		bool haveDefaultVideo = false;
		bool haveDefaultAudio = false;
		for (auto& [index, track] : m_tracks) {
			if (index != expected) {
				owner.Fail("mux output indexes must be contiguous from 0");
				return false;
			}
			++expected;
			AVStream* stream = avformat_new_stream(m_ctx, nullptr);
			if (!stream) {
				owner.Fail("avformat_new_stream failed");
				return false;
			}

			if (track.encoder) {
				if (!BindEncoderStream(owner, *track.encoder, stream)) {
					owner.Fail("encoder context is empty");
					return false;
				}
				if (owner.Language(index))
					track.language = owner.Language(index);
				if (owner.Title(index))
					track.title = owner.Title(index);
				av_dict_set(&stream->metadata, "ENCODER", track.encoder->EncoderTag().c_str(), 0);
			}
			else {
				if (!track.params) {
					owner.Fail("remux track has no codec parameters");
					return false;
				}
				if (avcodec_parameters_copy(stream->codecpar, track.params) < 0) {
					owner.Fail("avcodec_parameters_copy failed");
					return false;
				}
				if (track.srcTb.num > 0 && track.srcTb.den > 0)
					stream->time_base = track.srcTb;
			}

			if (track.language)
				av_dict_set(&stream->metadata, "language", track.language->c_str(), 0);
			if (track.title)
				av_dict_set(&stream->metadata, "title", track.title->c_str(), 0);

			if (stream->codecpar) {
				if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && !haveDefaultVideo) {
					stream->disposition |= AV_DISPOSITION_DEFAULT;
					haveDefaultVideo = true;
				}
				if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && !haveDefaultAudio) {
					stream->disposition |= AV_DISPOSITION_DEFAULT;
					haveDefaultAudio = true;
				}
			}

			track.avIndex = stream->index;
			track.timeBase = stream->time_base;
		}

		if (m_file && !Attachment::Write(owner, m_ctx, *m_file))
			return false;

		av_dict_set(&m_ctx->metadata, "encoding_tool", WritingApp, 0);

		bool hasAudio = false;
		bool hasVideo = false;
		for (unsigned i = 0; i < m_ctx->nb_streams; ++i) {
			const auto* par = m_ctx->streams[i]->codecpar;
			if (!par)
				continue;
			if (par->codec_type == AVMEDIA_TYPE_AUDIO)
				hasAudio = true;
			if (par->codec_type == AVMEDIA_TYPE_VIDEO)
				hasVideo = true;
		}
		if (hasAudio && hasVideo)
			m_ctx->max_interleave_delta = 120LL * 1000 * 1000;

		AVDictionary* opts = nullptr;
		av_dict_set(&opts, "default_mode", "passthrough", 0);
		const int rc = avformat_write_header(m_ctx, &opts);
		av_dict_free(&opts);
		if (rc < 0) {
			owner.Fail("avformat_write_header failed: " + AvError(rc));
			return false;
		}

		for (auto& [index, track] : m_tracks) {
			if (track.avIndex < 0 || track.avIndex >= static_cast<int>(m_ctx->nb_streams))
				continue;
			AVStream* stream = m_ctx->streams[track.avIndex];
			const AVRational tb = stream->time_base;
			if (tb.num > 0 && tb.den > 0)
				track.timeBase = tb;
			if (track.encoder)
				av_dict_set(&stream->metadata, "ENCODER", track.encoder->EncoderTag().c_str(), 0);
		}

		m_header = true;
		while (!m_queue.empty()) {
			std::shared_ptr<StormByte::Multimedia::Pipeline::Packet> queued = std::move(m_queue.front());
			m_queue.pop_front();
			if (!queued || !WritePacket(owner, *queued))
				return false;
		}
		return true;
	}

	bool Container::WritePacket(StormByte::Multimedia::Pipeline::Muxer& owner,
		StormByte::Multimedia::Pipeline::Packet& packet) noexcept {
		if (owner.Failed() || !m_header)
			return false;
		const int out = Resolve(packet.Track());
		const auto it = m_tracks.find(out);
		if (it == m_tracks.end() || it->second.avIndex < 0) {
			owner.Fail("packet stream index is not a mux track");
			return false;
		}
		AVPacket* raw = MakeAvPacket(packet);
		if (!raw) {
			owner.Fail("could not allocate output packet");
			return false;
		}
		auto& track = it->second;
		raw->stream_index = track.avIndex;
		const AVRational tb = track.timeBase;

		if (packet.Pts())
			raw->pts = NsToTicks(packet.Pts()->Nanoseconds().count(), tb);
		else
			raw->pts = AV_NOPTS_VALUE;
		if (packet.Dts())
			raw->dts = NsToTicks(packet.Dts()->Nanoseconds().count(), tb);
		else
			raw->dts = AV_NOPTS_VALUE;
		if (packet.Duration()) {
			const auto ticks = NsToTicks(packet.Duration()->Nanoseconds().count(), tb);
			raw->duration = (ticks == AV_NOPTS_VALUE || ticks < 0) ? 0 : ticks;
		}

		const bool isSub = packet.Type() == Type::Subtitle;
		if (isSub) {
			if (raw->dts == AV_NOPTS_VALUE)
				raw->dts = raw->pts;
		}
		else {
			if (raw->dts == AV_NOPTS_VALUE) {
				if (track.lastDts != AV_NOPTS_VALUE)
					raw->dts = track.lastDts + 1;
				else if (raw->pts != AV_NOPTS_VALUE)
					raw->dts = raw->pts;
			}
			else if (track.lastDts != AV_NOPTS_VALUE && raw->dts <= track.lastDts)
				raw->dts = track.lastDts + 1;
			if (raw->pts != AV_NOPTS_VALUE && raw->dts != AV_NOPTS_VALUE && raw->pts < raw->dts)
				raw->pts = raw->dts;
			if (raw->dts != AV_NOPTS_VALUE)
				track.lastDts = raw->dts;
		}

		const int rc = av_interleaved_write_frame(m_ctx, raw);
		av_packet_free(&raw);
		if (rc < 0) {
			owner.Fail("av_interleaved_write_frame failed: " + AvError(rc));
			return false;
		}
		return true;
	}

	void Container::Flush(StormByte::Multimedia::Pipeline::Muxer& owner) noexcept {
		if (owner.Failed() || m_trailer)
			return;
		for (const auto& [index, track] : m_tracks) {
			if (track.encoder && track.encoder->Failed()) {
				owner.Fail("encoder failed");
				return;
			}
		}

		for (auto& [index, track] : m_tracks) {
			if (!track.encoder || *track.encoder)
				continue;
			AVCodecParameters* params = avcodec_parameters_alloc();
			if (!params) {
				owner.Fail("avcodec_parameters_alloc failed");
				return;
			}
			params->codec_type = AVMEDIA_TYPE_SUBTITLE;
			params->codec_id = AV_CODEC_ID_SUBRIP;
			track.params = params;
			track.srcTb = AVRational{1, 1000};
			track.encoder = nullptr;
		}

		if (!WriteHeaderIfReady(owner))
			return;
		while (!m_queue.empty()) {
			std::shared_ptr<StormByte::Multimedia::Pipeline::Packet> leftover = std::move(m_queue.front());
			m_queue.pop_front();
			if (m_header) {
				if (!leftover || !WritePacket(owner, *leftover))
					return;
			}
		}
		if (!m_header)
			WriteHeaderIfReady(owner);
		if (m_header && !m_trailer && m_ctx) {
			av_dict_set(&m_ctx->metadata, "ENCODER", WritingApp, 0);
			av_write_trailer(m_ctx);
			m_trailer = true;
		}
	}

	void Container::Close() noexcept {
		if (!m_ctx)
			return;
		if (m_header && !m_trailer) {
			av_write_trailer(m_ctx);
			m_trailer = true;
		}
		if (m_ctx->pb && !(m_ctx->oformat && (m_ctx->oformat->flags & AVFMT_NOFILE)))
			avio_closep(&m_ctx->pb);
		avformat_free_context(m_ctx);
		m_ctx = nullptr;
	}
}
