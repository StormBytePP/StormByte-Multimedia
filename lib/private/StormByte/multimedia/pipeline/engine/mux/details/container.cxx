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
#include <StormByte/multimedia/pipeline/engine/copy/engine.hxx>
#include <StormByte/multimedia/pipeline/engine/mux/details/attachment.hxx>
#include <StormByte/multimedia/pipeline/engine/mux/details/container.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
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

namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;
namespace Details = StormByte::Multimedia::Pipeline::Engine::Mux::Details;

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
}

Details::Container::Container() noexcept = default;

Details::Container::~Container() noexcept {
	Close();
}

bool Details::Container::IsOpen() const noexcept {
	return m_ctx != nullptr;
}

bool Details::Container::HeaderWritten() const noexcept {
	return m_header;
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

bool Details::Container::BindPath(class Mux& owner, const std::filesystem::path& path) noexcept {
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

bool Details::Container::ReserveEncoder(class Mux& owner, class Encoder& encoder) noexcept {
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

bool Details::Container::ReserveCopy(class Mux& owner, const class Copy& copy) noexcept {
	if (m_header) {
		owner.Fail("cannot add a track after the header");
		return false;
	}
	if (!copy) {
		owner.Fail("copy input is not bound");
		return false;
	}
	if (copy.Index() < 0) {
		owner.Fail("copy output index is invalid");
		return false;
	}
	if (m_tracks.contains(copy.Index())) {
		owner.Fail("duplicate mux output index");
		return false;
	}
	if (m_inToOut.contains(copy.InputIndex())) {
		owner.Fail("duplicate mux copy input index");
		return false;
	}
	Track track;
	track.copy = &copy;
	track.inIndex = copy.InputIndex();
	m_tracks.emplace(copy.Index(), track);
	m_inToOut.emplace(copy.InputIndex(), copy.Index());
	return true;
}

bool Details::Container::BindAttachments(class Mux& owner, const File& file) noexcept {
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

bool Details::Container::WriteHeaderIfReady(class Mux& owner) noexcept {
	if (owner.Failed() || m_header)
		return !owner.Failed();
	if (!m_ctx)
		return true;
	if (m_tracks.empty())
		return true;
	for (const auto& [index, track] : m_tracks) {
		if (track.encoder) {
			if (track.encoder->Failed()) {
				owner.Fail(track.encoder->Error().value_or("encoder failed"));
				return false;
			}
			if (!*track.encoder)
				return true;
			continue;
		}
		if (track.copy) {
			if (!*track.copy)
				return true;
			continue;
		}
		owner.Fail("mux track has neither encoder nor copy");
		return false;
	}

	int expected = 0;
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
		}
		else {
			auto* par = track.copy->m_engine->params.Get();
			if (!par) {
				owner.Fail("copy stream has no codec parameters");
				return false;
			}
			if (avcodec_parameters_copy(stream->codecpar, par) < 0) {
				owner.Fail("avcodec_parameters_copy failed");
				return false;
			}
			AVRational tb = track.copy->m_engine->timeBase;
			if (tb.num <= 0 || tb.den <= 0)
				tb = AVRational{1, 1000};
			stream->time_base = tb;
			if (track.copy->m_engine->language)
				track.language = track.copy->m_engine->language;
			if (track.copy->m_engine->title)
				track.title = track.copy->m_engine->title;
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

	if (m_file && !Attachment::Write(owner, m_ctx, *m_file))
		return false;

	av_dict_set(&m_ctx->metadata, "encoding_tool", WritingApp, 0);

	AVDictionary* opts = nullptr;
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
		Packet queued = std::move(m_queue.front());
		m_queue.pop_front();
		if (!WritePacket(owner, queued))
			return false;
	}
	return true;
}

bool Details::Container::WritePacket(class Mux& owner, Packet& packet) noexcept {
	if (owner.Failed() || !m_header)
		return false;
	int out = packet.StreamIndex();
	if (!m_tracks.contains(out)) {
		const auto mapped = m_inToOut.find(out);
		out = (mapped == m_inToOut.end()) ? -1 : mapped->second;
	}
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

	const int rc = av_interleaved_write_frame(m_ctx, raw);
	av_packet_free(&raw);
	if (rc < 0) {
		owner.Fail("av_interleaved_write_frame failed: " + AvError(rc));
		return false;
	}
	return true;
}

bool Details::Container::Push(class Mux& owner, Packet& packet) noexcept {
	int out = packet.StreamIndex();
	if (!m_tracks.contains(out)) {
		const auto mapped = m_inToOut.find(out);
		out = (mapped == m_inToOut.end()) ? -1 : mapped->second;
	}
	if (out < 0) {
		owner.Fail("packet stream index is not a mux track");
		return false;
	}
	if (!m_ctx) {
		m_queue.push_back(std::move(packet));
		return true;
	}
	if (!WriteHeaderIfReady(owner))
		return false;
	if (m_header) {
		while (!m_queue.empty()) {
			Packet leftover = std::move(m_queue.front());
			m_queue.pop_front();
			if (!WritePacket(owner, leftover))
				return false;
		}
		return WritePacket(owner, packet);
	}
	m_queue.push_back(std::move(packet));
	return true;
}

void Details::Container::Flush(class Mux& owner) noexcept {
	if (owner.Failed() || m_trailer)
		return;
	for (const auto& [index, track] : m_tracks) {
		if (track.encoder && track.encoder->Failed()) {
			owner.Fail(track.encoder->Error().value_or("encoder failed"));
			return;
		}
	}
	if (!WriteHeaderIfReady(owner))
		return;
	for (auto& [index, track] : m_tracks) {
		if (!track.encoder || !*track.encoder)
			continue;
		track.encoder->Flush();
		Packet leftover;
		while (track.encoder->MuxTakePacket(leftover)) {
			if (m_header) {
				if (!WritePacket(owner, leftover))
					return;
			}
			else
				m_queue.push_back(std::move(leftover));
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
