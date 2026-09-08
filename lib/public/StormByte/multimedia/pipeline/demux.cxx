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
#include <StormByte/multimedia/backend/ffmpeg/AVStream.hxx>
#include <StormByte/multimedia/backend/ffmpeg/property.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/origin.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/details/audio.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/details/subtitle.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/details/video.hxx>
#include <StormByte/multimedia/pipeline/engine/demux/details/container.hxx>
#include <StormByte/multimedia/property/audio.hxx>
#include <StormByte/multimedia/property/video.hxx>

#include <variant>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia::Pipeline;
namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;

Demux::Demux() noexcept
: m_failed(false), m_eof(false) {}

Demux::Demux(Demux&&) noexcept = default;
Demux::~Demux() noexcept = default;
Demux& Demux::operator=(Demux&&) noexcept = default;

Demux::operator bool() const noexcept {
	return !m_failed && !m_eof && m_engine && m_engine->IsOpen();
}

bool Demux::Failed() const noexcept {
	return m_failed;
}

bool Demux::Eof() const noexcept {
	return m_eof;
}

void Demux::ReachedEof() noexcept {
	m_eof = true;
}

const std::optional<std::string>& Demux::Error() const noexcept {
	return m_error;
}

Filter::Chain::Packet& Demux::Pipe() noexcept {
	return m_pipe;
}

const Filter::Chain::Packet& Demux::Pipe() const noexcept {
	return m_pipe;
}

void Demux::Fail(std::string reason) noexcept {
	m_failed = true;
	m_error = std::move(reason);
	m_engine.reset();
	m_file = nullptr;
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

	auto engine = std::make_unique<Engine::Demux::Details::Container>();
	if (!engine->Adopt(demux, std::move(opened.value())))
		return demux;
	demux.m_engine = std::move(engine);
	demux.m_file = &file;
	demux.m_failed = false;
	demux.m_eof = false;
	demux.m_error.reset();
	return demux;
}

Demux& StormByte::Multimedia::Pipeline::operator>>(Demux& demux, class Packet& packet) noexcept {
	if (demux.m_failed || demux.m_eof)
		return demux;
	if (!demux.m_engine) {
		demux.Fail("demuxer is not open");
		return demux;
	}
	if (!demux.m_engine->Read(demux, packet))
		return demux;
	if (!demux.m_pipe.Push(packet)) {
		demux.Fail(demux.m_pipe.Error().value_or("packet filter failed"));
		packet = Packet{};
		return demux;
	}
	return demux;
}

Decoder& StormByte::Multimedia::Pipeline::operator>>(Demux& demux, Decoder& decoder) noexcept {
	if (decoder.Failed())
		return decoder;
	if (demux.m_failed || !demux.m_engine) {
		decoder.Fail(demux.m_error.value_or("demuxer is not open"));
		return decoder;
	}
	auto* container = static_cast<Engine::Demux::Details::Container*>(demux.m_engine.get());
	auto* fmt = container ? container->Format() : nullptr;
	if (!fmt) {
		decoder.Fail("demuxer is not open");
		return decoder;
	}

	std::optional<FFmpeg::AVCodecParameters> params;
	std::optional<StormByte::Multimedia::Stream::Properties> mapped;
	AVRational timeBase{0, 1};
	bool found = false;
	for (const auto& stream : fmt->Streams()) {
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

	if (demux.m_file) {
		for (const auto& stream : demux.m_file->Streams()) {
			if (stream.Index() != decoder.Index())
				continue;
			if (const auto& language = stream.Metadata().Language(); language)
				decoder.Language(*language);
			if (const auto& title = stream.Metadata().Title(); title)
				decoder.Title(*title);
			break;
		}
	}

	const ::AVCodec* codec = avcodec_find_decoder(static_cast<::AVCodecID>(params->CodecId()));
	auto backend = FFmpeg::AVDecoder::Open(
		const_cast<::AVCodec*>(codec), *params, *fmt, decoder.Index());
	if (!backend.has_value()) {
		decoder.Fail(backend.error()->what());
		return decoder;
	}

	std::unique_ptr<Engine::Decoder::Engine> engine;
	if (mapped.has_value() && std::holds_alternative<StormByte::Multimedia::Property::Video>(*mapped)) {
		engine = std::make_unique<Engine::Decoder::Details::Video>(
			std::move(backend.value()), timeBase,
			std::get<StormByte::Multimedia::Property::Video>(std::move(*mapped)));
	}
	else if (mapped.has_value() && std::holds_alternative<StormByte::Multimedia::Property::Audio>(*mapped)) {
		engine = std::make_unique<Engine::Decoder::Details::Audio>(
			std::move(backend.value()), timeBase,
			std::get<StormByte::Multimedia::Property::Audio>(std::move(*mapped)));
	}
	else {
		engine = std::make_unique<Engine::Decoder::Details::Subtitle>(
			std::move(backend.value()), timeBase);
	}
	decoder.Bind(std::move(engine));
	return decoder;
}
