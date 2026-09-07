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

#include <StormByte/multimedia/pipeline/copy.hxx>
#include <StormByte/multimedia/pipeline/copy_impl.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/encoder_impl.hxx>
#include <StormByte/multimedia/pipeline/mux.hxx>
#include <StormByte/multimedia/pipeline/mux_impl.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/type.hxx>

#include <cstdint>
#include <cstring>
#include <span>
#include <string>
#include <utility>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavcodec/packet.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libavutil/dict.h>
	#include <libavutil/error.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/mem.h>
	#include <libavutil/rational.h>
}

namespace StormByte::Multimedia::Pipeline {
	namespace {
		const AVRational NanoTimeBase{1, 1000000000};

		constexpr const char WritingApp[] = "StormByte-Multimedia " STORMBYTE_MULTIMEDIA_VERSION;

		std::int64_t NsToTicks(std::int64_t ns, AVRational time_base) noexcept {
			if (ns < 0 || time_base.num <= 0 || time_base.den <= 0)
				return AV_NOPTS_VALUE;
			return av_rescale_q(ns, NanoTimeBase, time_base);
		}

		std::string AvError(int err) noexcept {
			char buf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(err, buf, sizeof(buf));
			return buf;
		}

		AVPacket* MakeAvPacket(Packet& packet) noexcept {
			auto& fifo = packet.Payload();
			const auto size = fifo.AvailableBytes();
			StormByte::Buffer::DataType bytes;
			if (size > 0 && !fifo.Extract(size, bytes))
				return nullptr;
			AVPacket* raw = av_packet_alloc();
			if (!raw)
				return nullptr;
			if (!bytes.empty()) {
				if (av_new_packet(raw, static_cast<int>(bytes.size())) < 0) {
					av_packet_free(&raw);
					return nullptr;
				}
				std::memcpy(raw->data, bytes.data(), bytes.size());
			}
			if (packet.KeyFrame())
				raw->flags |= AV_PKT_FLAG_KEY;

			for (const auto& side : packet.Attachments()) {
				const auto& stored = side.Payload().Data();
				const auto avail = side.Payload().AvailableBytes();
				if (avail == 0 || avail > stored.size())
					continue;
				const auto* src = stored.data() + (stored.size() - avail);
				enum AVPacketSideDataType type = AV_PKT_DATA_NB;
				switch (side.Kind()) {
					case SideDataKind::HdrPlus:
						type = AV_PKT_DATA_DYNAMIC_HDR10_PLUS;
						break;
					case SideDataKind::MasteringDisplay:
						type = AV_PKT_DATA_MASTERING_DISPLAY_METADATA;
						break;
					case SideDataKind::ContentLight:
						type = AV_PKT_DATA_CONTENT_LIGHT_LEVEL;
						break;
					default:
						break;
				}
				if (type == AV_PKT_DATA_NB)
					continue;
				uint8_t* dst = av_packet_new_side_data(raw, type, static_cast<size_t>(avail));
				if (!dst) {
					av_packet_free(&raw);
					return nullptr;
				}
				std::memcpy(dst, src, avail);
			}
			return raw;
		}

		enum AVCodecID AttachmentCodecId(const std::optional<std::string>& mime) noexcept {
			if (!mime)
				return AV_CODEC_ID_NONE;
			if (*mime == "image/jpeg" || *mime == "image/jpg")
				return AV_CODEC_ID_MJPEG;
			if (*mime == "image/png")
				return AV_CODEC_ID_PNG;
			return AV_CODEC_ID_NONE;
		}

		std::span<const std::byte> UnreadSpan(const StormByte::Buffer::FIFO& fifo) noexcept {
			const auto& stored = fifo.Data();
			const auto avail = fifo.AvailableBytes();
			if (avail == 0 || avail > stored.size())
				return {};
			return std::span<const std::byte>{stored.data() + (stored.size() - avail), avail};
		}
	}

	Mux::Mux(const Container& container) noexcept
	: m_container(&container), m_impl(std::make_unique<Impl>()), m_failed(false) {
		if (!container.HasAccess(Access{Operation::Write}))
			Fail("container does not allow write");
	}

	Mux::Mux(Mux&& other) noexcept
	: m_container(other.m_container), m_impl(std::move(other.m_impl)), m_pipe(std::move(other.m_pipe)),
	m_failed(other.m_failed), m_error(std::move(other.m_error)) {
		other.m_failed = true;
	}

	Mux::~Mux() noexcept {
		Finish();
	}

	Mux& Mux::operator=(Mux&& other) noexcept {
		if (this == &other)
			return *this;
		Finish();
		m_container = other.m_container;
		m_impl = std::move(other.m_impl);
		m_pipe = std::move(other.m_pipe);
		m_failed = other.m_failed;
		m_error = std::move(other.m_error);
		other.m_failed = true;
		return *this;
	}

	Mux::operator bool() const noexcept {
		return !m_failed && m_impl && m_impl->m_ctx;
	}

	const Container& Mux::Destination() const noexcept {
		return *m_container;
	}

	bool Mux::Failed() const noexcept {
		return m_failed;
	}

	const std::optional<std::string>& Mux::Error() const noexcept {
		return m_error;
	}

	Filter::Pipe& Mux::Pipe() noexcept {
		return m_pipe;
	}

	const Filter::Pipe& Mux::Pipe() const noexcept {
		return m_pipe;
	}

	void Mux::Fail(std::string reason) noexcept {
		m_failed = true;
		m_error = std::move(reason);
		if (m_impl)
			m_impl->Close();
	}

	bool Mux::WriteAttachments() noexcept {
		if (m_failed || !m_impl || !m_impl->m_ctx)
			return !m_failed;
		if (!m_impl->m_file)
			return true;
		const auto& attachments = m_impl->m_file->Attachments();
		if (attachments.empty())
			return true;
		if (!m_container->HasAccess(Access{Operation::Attach})) {
			Fail("destination container does not support attachments");
			return false;
		}
		for (const auto& attachment : attachments) {
			AVStream* stream = avformat_new_stream(m_impl->m_ctx, nullptr);
			if (!stream) {
				Fail("avformat_new_stream failed for attachment");
				return false;
			}
			stream->codecpar->codec_type = AVMEDIA_TYPE_ATTACHMENT;
			stream->codecpar->codec_id = AttachmentCodecId(attachment.MimeType());
			if (attachment.FileName())
				av_dict_set(&stream->metadata, "filename", attachment.FileName()->c_str(), 0);
			if (attachment.MimeType())
				av_dict_set(&stream->metadata, "mimetype", attachment.MimeType()->c_str(), 0);

			const auto view = UnreadSpan(attachment.Payload());
			if (view.empty())
				continue;
			auto* extra = static_cast<std::uint8_t*>(
				av_malloc(view.size() + static_cast<std::size_t>(AV_INPUT_BUFFER_PADDING_SIZE)));
			if (!extra) {
				Fail("av_malloc failed for attachment");
				return false;
			}
			std::memcpy(extra, view.data(), view.size());
			std::memset(extra + view.size(), 0, static_cast<std::size_t>(AV_INPUT_BUFFER_PADDING_SIZE));
			stream->codecpar->extradata = extra;
			stream->codecpar->extradata_size = static_cast<int>(view.size());
		}
		return true;
	}

	bool Mux::WriteHeaderIfReady() noexcept {
		if (m_failed || !m_impl || m_impl->m_header)
			return !m_failed;
		if (!m_impl->m_ctx)
			return true;
		if (m_impl->m_tracks.empty())
			return true;
		for (const auto& [index, track] : m_impl->m_tracks) {
			if (track.encoder) {
				if (!track.encoder->m_impl)
					return true;
				continue;
			}
			if (track.copy) {
				if (!track.copy->m_impl || !track.copy->m_impl->bound)
					return true;
				continue;
			}
			Fail("mux track has neither encoder nor copy");
			return false;
		}

		int expected = 0;
		for (auto& [index, track] : m_impl->m_tracks) {
			if (index != expected) {
				Fail("mux output indexes must be contiguous from 0");
				return false;
			}
			++expected;
			AVStream* stream = avformat_new_stream(m_impl->m_ctx, nullptr);
			if (!stream) {
				Fail("avformat_new_stream failed");
				return false;
			}

			if (track.encoder) {
				auto* ctx = track.encoder->m_impl->m_encoder.Get();
				if (!ctx) {
					Fail("encoder context is empty");
					return false;
				}
				if (avcodec_parameters_from_context(stream->codecpar, ctx) < 0) {
					Fail("avcodec_parameters_from_context failed");
					return false;
				}
				AVRational tb = track.encoder->m_impl->m_encoder.TimeBase();
				if (tb.num <= 0 || tb.den <= 0)
					tb = ctx->time_base;
				if (tb.num <= 0 || tb.den <= 0)
					tb = AVRational{1, 1000};
				stream->time_base = tb;
				if (track.encoder->Language())
					track.language = track.encoder->Language();
				if (track.encoder->Title())
					track.title = track.encoder->Title();
			}
			else {
				auto* par = track.copy->m_impl->params.Get();
				if (!par) {
					Fail("copy stream has no codec parameters");
					return false;
				}
				if (avcodec_parameters_copy(stream->codecpar, par) < 0) {
					Fail("avcodec_parameters_copy failed");
					return false;
				}
				AVRational tb = track.copy->m_impl->timeBase;
				if (tb.num <= 0 || tb.den <= 0)
					tb = AVRational{1, 1000};
				stream->time_base = tb;
				if (track.copy->m_impl->language)
					track.language = track.copy->m_impl->language;
				if (track.copy->m_impl->title)
					track.title = track.copy->m_impl->title;
			}

			if (track.language)
				av_dict_set(&stream->metadata, "language", track.language->c_str(), 0);
			if (track.title)
				av_dict_set(&stream->metadata, "title", track.title->c_str(), 0);
			if (track.encoder)
				av_dict_set(&stream->metadata, "ENCODER", track.encoder->EncoderTag().c_str(), 0);

			track.avIndex = stream->index;
			track.timeBase = stream->time_base;
		}

		if (!WriteAttachments())
			return false;

		av_dict_set(&m_impl->m_ctx->metadata, "encoding_tool", WritingApp, 0);

		AVDictionary* opts = nullptr;
		const int rc = avformat_write_header(m_impl->m_ctx, &opts);
		av_dict_free(&opts);
		if (rc < 0) {
			Fail("avformat_write_header failed: " + AvError(rc));
			return false;
		}

		for (auto& [index, track] : m_impl->m_tracks) {
			if (track.avIndex < 0 || track.avIndex >= static_cast<int>(m_impl->m_ctx->nb_streams))
				continue;
			AVStream* stream = m_impl->m_ctx->streams[track.avIndex];
			const AVRational tb = stream->time_base;
			if (tb.num > 0 && tb.den > 0)
				track.timeBase = tb;
			if (track.encoder)
				av_dict_set(&stream->metadata, "ENCODER", track.encoder->EncoderTag().c_str(), 0);
		}

		m_impl->m_header = true;
		while (!m_impl->m_queue.empty()) {
			Packet queued = std::move(m_impl->m_queue.front());
			m_impl->m_queue.pop_front();
			if (!WritePacket(queued))
				return false;
		}
		return true;
	}

	bool Mux::WritePacket(Packet& packet) noexcept {
		if (m_failed || !m_impl || !m_impl->m_header)
			return false;
		int out = packet.StreamIndex();
		if (!m_impl->m_tracks.contains(out)) {
			const auto mapped = m_impl->m_inToOut.find(out);
			out = (mapped == m_impl->m_inToOut.end()) ? -1 : mapped->second;
		}
		const auto it = m_impl->m_tracks.find(out);
		if (it == m_impl->m_tracks.end() || it->second.avIndex < 0) {
			Fail("packet stream index is not a mux track");
			return false;
		}
		AVPacket* raw = MakeAvPacket(packet);
		if (!raw) {
			Fail("could not allocate output packet");
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
			raw->dts = raw->pts;
		if (raw->dts == AV_NOPTS_VALUE)
			raw->dts = raw->pts;
		if (packet.Duration()) {
			const auto ticks = NsToTicks(packet.Duration()->Nanoseconds().count(), tb);
			raw->duration = (ticks == AV_NOPTS_VALUE) ? 0 : ticks;
		}
		if (raw->dts != AV_NOPTS_VALUE && track.lastDts != AV_NOPTS_VALUE && raw->dts < track.lastDts)
			raw->dts = track.lastDts;
		if (raw->pts != AV_NOPTS_VALUE && raw->dts != AV_NOPTS_VALUE && raw->pts < raw->dts)
			raw->pts = raw->dts;
		if (raw->dts != AV_NOPTS_VALUE)
			track.lastDts = raw->dts + (raw->duration > 0 ? raw->duration : 1);

		const int rc = av_interleaved_write_frame(m_impl->m_ctx, raw);
		av_packet_free(&raw);
		if (rc < 0) {
			Fail("av_interleaved_write_frame failed: " + AvError(rc));
			return false;
		}
		return true;
	}

	void Mux::Flush() noexcept {
		if (m_failed || !m_impl || m_impl->m_trailer)
			return;
		if (!WriteHeaderIfReady())
			return;
		for (auto& [index, track] : m_impl->m_tracks) {
			if (!track.encoder || !track.encoder->m_impl)
				continue;
			track.encoder->Flush();
			while (!track.encoder->m_impl->m_pending.empty()) {
				Packet leftover = std::move(track.encoder->m_impl->m_pending.front());
				track.encoder->m_impl->m_pending.pop_front();
				if (m_impl->m_header) {
					if (!WritePacket(leftover))
						return;
				}
				else
					m_impl->m_queue.push_back(std::move(leftover));
			}
		}
		if (!m_impl->m_header)
			WriteHeaderIfReady();
		if (m_impl->m_header && !m_impl->m_trailer && m_impl->m_ctx) {
			av_dict_set(&m_impl->m_ctx->metadata, "ENCODER", WritingApp, 0);
			av_write_trailer(m_impl->m_ctx);
			m_impl->m_trailer = true;
		}
	}

	void Mux::Finish() noexcept {
		if (!m_impl)
			return;
		if (!m_failed)
			Flush();
		m_impl->Close();
	}

	Encoder& operator>>(Encoder& encoder, Mux& mux) noexcept {
		if (mux.m_failed || encoder.Failed())
			return encoder;
		if (!mux.m_impl) {
			mux.Fail("muxer is not open");
			return encoder;
		}
		if (mux.m_impl->m_header) {
			mux.Fail("cannot add a track after the header");
			return encoder;
		}
		if (encoder.Index() < 0) {
			mux.Fail("encoder output index is invalid");
			return encoder;
		}
		if (mux.m_impl->m_tracks.contains(encoder.Index())) {
			mux.Fail("duplicate mux output index");
			return encoder;
		}
		Mux::Impl::Track track;
		track.encoder = &encoder;
		mux.m_impl->m_tracks.emplace(encoder.Index(), track);
		return encoder;
	}

	Mux& operator>>(Mux& mux, const std::filesystem::path& path) noexcept {
		if (mux.m_failed)
			return mux;
		if (!mux.m_impl) {
			mux.Fail("muxer has no backend");
			return mux;
		}
		if (mux.m_impl->m_ctx) {
			mux.Fail("muxer destination is already bound");
			return mux;
		}
		if (path.empty()) {
			mux.Fail("mux destination path is empty");
			return mux;
		}
		const auto ext = mux.m_container->Extension();
		const std::string dummy = ext.empty() ? path.string() : ("out." + std::string(ext));
		const AVOutputFormat* oformat = av_guess_format(nullptr, dummy.c_str(), nullptr);
		if (!oformat) {
			mux.Fail("could not guess output format from container");
			return mux;
		}
		AVFormatContext* ctx = nullptr;
		if (avformat_alloc_output_context2(&ctx, const_cast<AVOutputFormat*>(oformat), nullptr, path.c_str()) < 0 || !ctx) {
			mux.Fail("avformat_alloc_output_context2 failed");
			return mux;
		}
		mux.m_impl->m_ctx = ctx;
		mux.m_impl->m_path = path;
		if (!(ctx->oformat->flags & AVFMT_NOFILE)) {
			if (avio_open(&ctx->pb, path.c_str(), AVIO_FLAG_WRITE) < 0) {
				mux.Fail("avio_open failed");
				return mux;
			}
		}
		if (!mux.WriteHeaderIfReady())
			return mux;
		while (mux.m_impl && mux.m_impl->m_header && !mux.m_impl->m_queue.empty()) {
			Packet leftover = std::move(mux.m_impl->m_queue.front());
			mux.m_impl->m_queue.pop_front();
			if (!mux.WritePacket(leftover))
				return mux;
		}
		return mux;
	}

	Mux& operator>>(const File& file, Mux& mux) noexcept {
		if (mux.m_failed)
			return mux;
		if (!mux.m_impl) {
			mux.Fail("muxer has no backend");
			return mux;
		}
		if (mux.m_impl->m_header) {
			mux.Fail("cannot bind attachments after the header");
			return mux;
		}
		mux.m_impl->m_file = &file;
		if (!file.Attachments().empty() && !mux.m_container->HasAccess(Access{Operation::Attach}))
			mux.Fail("destination container does not support attachments");
		return mux;
	}

	Mux& operator>>(Demux& demux, Mux& mux) noexcept {
		if (mux.m_failed)
			return mux;
		if (!demux.m_file) {
			mux.Fail("demuxer has no source file for attachments");
			return mux;
		}
		return operator>>(*demux.m_file, mux);
	}

	Packet& operator>>(Packet& packet, Mux& mux) noexcept {
		if (mux.m_failed)
			return packet;
		if (!mux.m_impl) {
			mux.Fail("muxer has no backend");
			return packet;
		}
		auto filtered = mux.m_pipe.Push(std::move(packet));
		if (mux.m_pipe.Failed()) {
			mux.Fail(mux.m_pipe.Error().value_or("mux packet pipe failed"));
			return packet;
		}
		if (!filtered)
			return packet;
		packet = std::move(*filtered);
		int out = packet.StreamIndex();
		if (!mux.m_impl->m_tracks.contains(out)) {
			const auto mapped = mux.m_impl->m_inToOut.find(out);
			out = (mapped == mux.m_impl->m_inToOut.end()) ? -1 : mapped->second;
		}
		if (out < 0) {
			mux.Fail("packet stream index is not a mux track");
			return packet;
		}
		mux.m_impl->m_queue.push_back(std::move(packet));
		if (!mux.m_impl->m_ctx)
			return packet;
		if (!mux.WriteHeaderIfReady())
			return packet;
		if (mux.m_impl->m_header && !mux.m_impl->m_queue.empty()) {
			Packet leftover = std::move(mux.m_impl->m_queue.front());
			mux.m_impl->m_queue.pop_front();
			mux.WritePacket(leftover);
		}
		return packet;
	}
}
