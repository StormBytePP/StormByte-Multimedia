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

#include <StormByte/multimedia/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVDecoder.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVStream.hxx>
#include <StormByte/multimedia/backend/ffmpeg/property.hxx>
#include <StormByte/multimedia/detail/cover.hxx>
#include <StormByte/multimedia/features.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/origin.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/decoder_impl.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/demux_impl.hxx>
#include <StormByte/multimedia/property/audio.hxx>
#include <StormByte/multimedia/property/video.hxx>
#include <StormByte/multimedia/registry.hxx>
#include <tables/decoder/table.hxx>

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <variant>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavcodec/packet.h>
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/rational.h>
}

using namespace StormByte::Multimedia::Pipeline;
namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;

namespace {
	std::optional<StormByte::Multimedia::Property::Duration> TicksToPts(std::int64_t ticks, AVRational timeBase) noexcept {
		if (ticks == AV_NOPTS_VALUE || ticks < 0 || timeBase.num <= 0 || timeBase.den <= 0)
			return std::nullopt;
		const std::int64_t ns = av_rescale_q(ticks, timeBase, AVRational{1, 1000000000});
		if (ns < 0)
			return std::nullopt;
		return StormByte::Multimedia::Property::Duration{std::chrono::nanoseconds{ns}};
	}

	std::optional<StormByte::Multimedia::Property::Duration> TicksToDuration(std::int64_t ticks, AVRational timeBase) noexcept {
		if (ticks == AV_NOPTS_VALUE || ticks <= 0 || timeBase.num <= 0 || timeBase.den <= 0)
			return std::nullopt;
		const std::int64_t ns = av_rescale_q(ticks, timeBase, AVRational{1, 1000000000});
		if (ns <= 0)
			return std::nullopt;
		return StormByte::Multimedia::Property::Duration{std::chrono::nanoseconds{ns}};
	}

	StormByte::Multimedia::Features StreamNeed(
		const StormByte::Multimedia::Features& extra,
		const std::optional<StormByte::Multimedia::Property::Video>& video) noexcept {
		StormByte::Multimedia::Features need = extra;
		if (!video || !video->HDR10())
			return need;
		need.Add(StormByte::Multimedia::Feature::HDR10);
		if (video->HDR10()->IsHDR10Plus()) {
			need.Add(StormByte::Multimedia::Feature::HDR10Plus);
			need.Add(StormByte::Multimedia::Feature::SideData);
		}
		return need;
	}

	const StormByte::Multimedia::Tables::Decoder::DecoderDef* FindRow(
		std::string_view codec, std::string_view pin) noexcept {
		auto scan = [&](std::span<const StormByte::Multimedia::Tables::Decoder::DecoderDef> rows)
			-> const StormByte::Multimedia::Tables::Decoder::DecoderDef* {
			for (const auto& row : rows) {
				if (codec != row.codec)
					continue;
				if (!pin.empty() && pin != row.name)
					continue;
				if (pin.empty())
					continue;
				return &row;
			}
			return nullptr;
		};
		if (const auto* row = scan(StormByte::Multimedia::Tables::Decoder::Video()))
			return row;
		return scan(StormByte::Multimedia::Tables::Decoder::Audio());
	}

	const StormByte::Multimedia::Tables::Decoder::DecoderDef* PickDecoder(
		std::string_view codec, std::string_view pin, const StormByte::Multimedia::Features& need) noexcept {
		auto scan = [&](std::span<const StormByte::Multimedia::Tables::Decoder::DecoderDef> rows)
			-> const StormByte::Multimedia::Tables::Decoder::DecoderDef* {
			const StormByte::Multimedia::Tables::Decoder::DecoderDef* best = nullptr;
			for (const auto& row : rows) {
				if (codec != row.codec)
					continue;
				if (!pin.empty() && pin != row.name)
					continue;
				if (!row.features.Has(need))
					continue;
				if (avcodec_find_decoder_by_name(row.name) == nullptr)
					continue;
				if (!best || row.preference < best->preference)
					best = &row;
			}
			return best;
		};
		if (const auto* row = scan(StormByte::Multimedia::Tables::Decoder::Video()))
			return row;
		return scan(StormByte::Multimedia::Tables::Decoder::Audio());
	}
}

Demux::Demux() noexcept
: m_failed(false), m_eof(false) {}

Demux::Demux(Demux&&) noexcept = default;
Demux::~Demux() noexcept = default;
Demux& Demux::operator=(Demux&&) noexcept = default;

Demux::operator bool() const noexcept {
	return !m_failed && !m_eof && static_cast<bool>(m_impl);
}

bool Demux::Failed() const noexcept {
	return m_failed;
}

bool Demux::Eof() const noexcept {
	return m_eof;
}

const std::optional<std::string>& Demux::Error() const noexcept {
	return m_error;
}

Filter::Pipe& Demux::Pipe() noexcept {
	return m_pipe;
}

const Filter::Pipe& Demux::Pipe() const noexcept {
	return m_pipe;
}

void Demux::Fail(std::string reason) noexcept {
	m_failed = true;
	m_error = std::move(reason);
	m_impl.reset();
}

Demux& StormByte::Multimedia::Pipeline::operator>>(const File& file, Demux& demux) noexcept {
	if (demux.m_failed)
		return demux;

	auto opened = file.m_origin->Visit([](auto&& held) {
		return FFmpeg::AVFormatContext::Open(held);
	});
	if (!opened.has_value()) {
		demux.Fail(opened.error()->what());
		return demux;
	}

	demux.m_impl = std::make_unique<Demux::Impl>(std::move(opened.value()));
	demux.m_failed = false;
	demux.m_eof = false;
	demux.m_error.reset();
	return demux;
}

Demux& StormByte::Multimedia::Pipeline::operator>>(Demux& demux, Packet& packet) noexcept {
	if (demux.m_failed || demux.m_eof)
		return demux;
	if (!demux.m_impl) {
		demux.Fail("demuxer is not open");
		return demux;
	}

	for (;;) {
		const auto result = demux.m_impl->m_ctx.ReadPacket(demux.m_impl->m_scratch);
		if (result == FFmpeg::OperationResult::EndOfFile) {
			demux.m_eof = true;
			return demux;
		}
		if (result == FFmpeg::OperationResult::TryAgain)
			continue;
		if (result != FFmpeg::OperationResult::Success) {
			demux.Fail("failed to read packet");
			return demux;
		}

		const int index = demux.m_impl->m_scratch.StreamIndex();
		if (StormByte::Multimedia::Detail::IsAttachmentIndex(demux.m_impl->m_ctx, index)) {
			demux.m_impl->m_scratch.Unref();
			continue;
		}

		AVRational tb{0, 1};
		if (const auto found = demux.m_impl->m_timeBase.find(index); found != demux.m_impl->m_timeBase.end())
			tb = found->second;

		StormByte::Buffer::DataType bytes;
		const auto* data = demux.m_impl->m_scratch.Data();
		const int size = demux.m_impl->m_scratch.Size();
		if (data && size > 0) {
			const auto* raw = reinterpret_cast<const std::byte*>(data);
			bytes.assign(raw, raw + size);
		}

		Packet raw{
			index,
			StormByte::Buffer::FIFO{std::move(bytes)},
			TicksToPts(demux.m_impl->m_scratch.Pts(), tb),
			TicksToPts(demux.m_impl->m_scratch.Dts(), tb),
			TicksToDuration(demux.m_impl->m_scratch.Duration(), tb),
			(demux.m_impl->m_scratch.Flags() & AV_PKT_FLAG_KEY) != 0
		};
		demux.m_impl->m_scratch.Unref();

		auto filtered = demux.m_pipe.Push(std::move(raw));
		if (demux.m_pipe.Failed()) {
			demux.Fail(demux.m_pipe.Error().value_or("packet filter failed"));
			return demux;
		}
		if (!filtered.has_value())
			continue;
		packet = std::move(*filtered);
		return demux;
	}
}

Decoder& StormByte::Multimedia::Pipeline::operator>>(Demux& demux, Decoder& decoder) noexcept {
	if (decoder.Failed())
		return decoder;
	if (demux.m_failed || !demux.m_impl) {
		decoder.Fail(demux.m_error.value_or("demuxer is not open"));
		return decoder;
	}

	if (StormByte::Multimedia::Detail::IsAttachmentIndex(demux.m_impl->m_ctx, decoder.Index())) {
		decoder.Fail("stream is a container attachment");
		return decoder;
	}

	std::optional<FFmpeg::AVCodecParameters> params;
	std::optional<StormByte::Multimedia::Stream::Properties> mapped;
	AVRational timeBase{0, 1};
	bool found = false;
	for (const auto& stream : demux.m_impl->m_ctx.Streams()) {
		if (stream.Index() != decoder.Index())
			continue;
		params = stream.CodecParameters();
		mapped = FFmpeg::MapProperties(stream);
		timeBase = stream.TimeBase();
		found = true;
		break;
	}
	if (!found || !params.has_value()) {
		decoder.Fail("stream index out of range");
		return decoder;
	}

	std::optional<StormByte::Multimedia::Property::Video> video;
	std::optional<StormByte::Multimedia::Property::Audio> audio;
	if (mapped.has_value() && std::holds_alternative<StormByte::Multimedia::Property::Video>(*mapped))
		video = std::get<StormByte::Multimedia::Property::Video>(std::move(*mapped));
	else if (mapped.has_value() && std::holds_alternative<StormByte::Multimedia::Property::Audio>(*mapped))
		audio = std::get<StormByte::Multimedia::Property::Audio>(std::move(*mapped));

	const auto need = StreamNeed(decoder.Require(), video);
	const char* ffmpegName = avcodec_get_name(static_cast<::AVCodecID>(params->CodecId()));
	std::string stormName;
	if (ffmpegName) {
		auto codec = StormByte::Multimedia::Registry::Instance().FindCodec(ffmpegName);
		if (codec.has_value())
			stormName = std::string(codec.value().get().Name());
	}

	const std::string_view pin = decoder.Implementation()
		? std::string_view{*decoder.Implementation()} : std::string_view{};

	if (!pin.empty()) {
		if (avcodec_find_decoder_by_name(std::string(pin).c_str()) == nullptr) {
			decoder.Fail("decoder implementation is unavailable");
			return decoder;
		}
		const auto* listed = stormName.empty() ? nullptr : FindRow(stormName, pin);
		if (!listed || !listed->features.Has(need)) {
			decoder.Fail("decoder implementation lacks required features");
			return decoder;
		}
	}

	const auto* row = stormName.empty() ? nullptr : PickDecoder(stormName, pin, need);

	const ::AVCodec* codec = nullptr;
	if (row)
		codec = avcodec_find_decoder_by_name(row->name);
	if (!codec)
		codec = avcodec_find_decoder(static_cast<::AVCodecID>(params->CodecId()));
	if (!codec) {
		decoder.Fail("no decoder for stream codec");
		return decoder;
	}

	auto backend = FFmpeg::AVDecoder::Open(
		const_cast<::AVCodec*>(codec), *params, demux.m_impl->m_ctx, decoder.Index());
	if (!backend.has_value()) {
		decoder.Fail(backend.error()->what());
		return decoder;
	}

	auto impl = std::make_unique<Decoder::Impl>(std::move(backend.value()));
	impl->m_timeBase = timeBase;
	impl->m_video = std::move(video);
	impl->m_audio = std::move(audio);
	if (row) {
		decoder.Implementation(row->name);
		decoder.m_capabilities = row->features;
	}
	else
		decoder.m_capabilities = StormByte::Multimedia::Features{};
	decoder.Bind(std::move(impl));
	return decoder;
}
