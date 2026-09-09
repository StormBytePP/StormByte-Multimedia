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

#include <StormByte/multimedia/pipeline/engine/mux/details/attachment.hxx>
#include <StormByte/multimedia/pipeline/engine/mux/details/container.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/engine/demux/engine.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/type.hxx>

#ifdef WINDOWS
#include <StormByte/string.hxx>
#endif

#include <cstdint>
#include <cstdio>
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

namespace Details = StormByte::Multimedia::Pipeline::Engine::Mux::Details;

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

	char TypeTag(StormByte::Multimedia::Type type) noexcept {
		switch (type) {
			case StormByte::Multimedia::Type::Video: return 'v';
			case StormByte::Multimedia::Type::Audio: return 'a';
			case StormByte::Multimedia::Type::Subtitle: return 's';
			default: return '?';
		}
	}
}

Details::Container::Container() noexcept
: m_ctx(nullptr), m_file(nullptr), m_header(false), m_trailer(false) {}

Details::Container::~Container() noexcept {
	FreeParams();
	Close();
}

void Details::Container::FreeParams() noexcept {
	for (auto& [index, track] : m_tracks) {
		if (track.params) {
			avcodec_parameters_free(&track.params);
			track.params = nullptr;
		}
	}
}

bool Details::Container::IsOpen() const noexcept {
	return m_ctx != nullptr;
}

bool Details::Container::HeaderWritten() const noexcept {
	return m_header;
}

int Details::Container::Resolve(int track) const noexcept {
	if (m_tracks.contains(track))
		return track;
	const auto it = m_inToOut.find(track);
	if (it != m_inToOut.end())
		return it->second;
	return -1;
}

bool Details::Container::BindPath(class StormByte::Multimedia::Pipeline::Mux& owner,
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

bool Details::Container::ReserveEncoder(class StormByte::Multimedia::Pipeline::Mux& owner,
	class StormByte::Multimedia::Pipeline::Encoder& encoder) noexcept {
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
	m_tracks.emplace(encoder.Index(), track);
	return true;
}

bool Details::Container::Remux(class StormByte::Multimedia::Pipeline::Mux& owner,
	class StormByte::Multimedia::Pipeline::Demux& demux, int in, int out) noexcept {
	if (m_header) {
		owner.Fail("cannot add a track after the header");
		return false;
	}
	if (out < 0) {
		owner.Fail("mux output index is invalid");
		return false;
	}
	if (m_tracks.contains(out)) {
		owner.Fail("duplicate mux output index");
		return false;
	}
	if (!demux.m_engine || !demux.m_engine->Context()) {
		owner.Fail("demuxer has no format context");
		return false;
	}
	auto* ctx = static_cast<AVFormatContext*>(demux.m_engine->Context());
	if (in < 0 || in >= static_cast<int>(ctx->nb_streams) || !ctx->streams[in] || !ctx->streams[in]->codecpar) {
		owner.Fail("copy source stream is invalid");
		return false;
	}
	AVCodecParameters* params = avcodec_parameters_alloc();
	if (!params) {
		owner.Fail("avcodec_parameters_alloc failed");
		return false;
	}
	if (avcodec_parameters_copy(params, ctx->streams[in]->codecpar) < 0) {
		avcodec_parameters_free(&params);
		owner.Fail("avcodec_parameters_copy failed");
		return false;
	}
	Track track;
	track.inIndex = in;
	track.params = params;
	track.srcTb = ctx->streams[in]->time_base;
	if (AVDictionaryEntry* lang = av_dict_get(ctx->streams[in]->metadata, "language", nullptr, 0))
		track.language = lang->value;
	if (AVDictionaryEntry* title = av_dict_get(ctx->streams[in]->metadata, "title", nullptr, 0))
		track.title = title->value;
	m_tracks.emplace(out, track);
	m_inToOut.emplace(in, out);
	return true;
}

bool Details::Container::BindAttachments(class StormByte::Multimedia::Pipeline::Mux& owner,
	const File& file) noexcept {
	if (m_header) {
		owner.Fail("cannot bind attachments after the header");
		return false;
	}
	m_file = &file;
	if (!file.Attachments().empty() && !owner.Destination().HasAccess(Access{Operation::Attach})) {
		owner.Fail("destination container does not support attachments");
		return false;
	}
	return true;
}

bool Details::Container::Push(class StormByte::Multimedia::Pipeline::Mux& owner,
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

bool Details::Container::WriteHeaderIfReady(class StormByte::Multimedia::Pipeline::Mux& owner) noexcept {
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
			if (!track.encoder->MuxBindStream(stream)) {
				owner.Fail("encoder context is empty");
				return false;
			}
			if (track.encoder->Language())
				track.language = track.encoder->Language();
			if (track.encoder->Title())
				track.title = track.encoder->Title();
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
	std::fprintf(stderr, "STMM-M header streams=%u a=%d v=%d interleave=%lld\n",
		m_ctx->nb_streams,
		hasAudio ? 1 : 0,
		hasVideo ? 1 : 0,
		static_cast<long long>(m_ctx->max_interleave_delta));
	std::fflush(stderr);

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

bool Details::Container::WritePacket(class StormByte::Multimedia::Pipeline::Mux& owner,
	class StormByte::Multimedia::Pipeline::Packet& packet) noexcept {
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

	const bool isVideo = packet.Type() == StormByte::Multimedia::Type::Video;
	const bool isAudio = packet.Type() == StormByte::Multimedia::Type::Audio;
	const bool isSub = packet.Type() == StormByte::Multimedia::Type::Subtitle;
	const std::int64_t inPtsNs = packet.Pts() ? packet.Pts()->Nanoseconds().count() : -1;
	const std::int64_t inDtsNs = packet.Dts() ? packet.Dts()->Nanoseconds().count() : -1;

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

	const std::int64_t dtsBeforeFix = raw->dts;
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

	static int nv = 0, na = 0, ns = 0, nAll = 0;
	++nAll;
	const bool logThis = (isVideo && nv < 24) || (isAudio && na < 24) || (isSub && ns < 24);
	if (logThis) {
		int& n = isVideo ? nv : (isAudio ? na : ns);
		std::fprintf(stderr,
			"STMM-M %c n=%d track=%d av=%d key=%d "
			"in_pts_ns=%lld in_dts_ns=%lld "
			"tb=%d/%d ticks_pts=%lld ticks_dts_raw=%lld ticks_dts=%lld last=%lld dur=%lld bytes=%d\n",
			TypeTag(packet.Type()),
			n,
			packet.Track(),
			track.avIndex,
			packet.KeyFrame() ? 1 : 0,
			static_cast<long long>(inPtsNs),
			static_cast<long long>(inDtsNs),
			tb.num, tb.den,
			static_cast<long long>(raw->pts),
			static_cast<long long>(dtsBeforeFix),
			static_cast<long long>(raw->dts),
			static_cast<long long>(track.lastDts),
			static_cast<long long>(raw->duration),
			raw->size);
		std::fflush(stderr);
		++n;
	}
	if (nAll == 1 || nAll == 50 || nAll == 200 || (nAll % 500) == 0) {
		std::fprintf(stderr, "STMM-M mix n=%d last=%c av=%d dts_ms=%lld\n",
			nAll,
			TypeTag(packet.Type()),
			track.avIndex,
			raw->dts == AV_NOPTS_VALUE ? -1LL : static_cast<long long>(raw->dts));
		std::fflush(stderr);
	}

	const int rc = av_interleaved_write_frame(m_ctx, raw);
	av_packet_free(&raw);
	if (rc < 0) {
		owner.Fail("av_interleaved_write_frame failed: " + AvError(rc));
		return false;
	}
	return true;
}

void Details::Container::Flush(class StormByte::Multimedia::Pipeline::Mux& owner) noexcept {
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
		std::fprintf(stderr, "STMM-M drop idle encoder out=%d (never opened)\n", index);
		std::fflush(stderr);
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
		std::fprintf(stderr, "STMM-M trailer written\n");
		std::fflush(stderr);
	}
	else if (!m_header) {
		std::fprintf(stderr, "STMM-M flush without header\n");
		std::fflush(stderr);
	}
}

void Details::Container::Close() noexcept {
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
