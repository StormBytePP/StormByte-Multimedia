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
#include <StormByte/multimedia/buffer/sink.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/origin.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/details/audio.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/details/subtitle.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/details/video.hxx>
#include <StormByte/multimedia/pipeline/engine/demux/details/container.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/plan.hxx>
#include <StormByte/multimedia/property/audio.hxx>
#include <StormByte/multimedia/property/video.hxx>
#include <StormByte/multimedia/name_thread.hxx>

#include <memory>
#include <variant>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia::Pipeline;
namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;

Demux::Demux() noexcept
: Step(Kinds{}, Kinds{Kind::Packet}), m_eof(false), m_positionNs(-1) {
	Launch();
}

Demux::~Demux() noexcept = default;

Demux::operator bool() const noexcept {
	return !Failed() && !m_eof && m_engine && m_engine->IsOpen();
}

bool Demux::Eof() const noexcept {
	return m_eof;
}

std::optional<StormByte::Multimedia::Property::Duration> Demux::Position() const noexcept {
	const std::int64_t ns = m_positionNs.load(std::memory_order_acquire);
	if (ns < 0)
		return std::nullopt;
	return StormByte::Multimedia::Property::Duration{std::chrono::nanoseconds{ns}};
}

void Demux::ReachedEof() noexcept {
	m_eof = true;
}

void Demux::Fail(std::string reason) noexcept {
	m_ready.notify_all();
	Step::Fail(std::move(reason));
}

void Demux::Pump() noexcept {
	NameThread("STMM:Demux");
	{
		std::unique_lock lock(m_readyMutex);
		m_ready.wait(lock, [this]() {
			return Failed() || static_cast<bool>(m_plan);
		});
	}
	if (Failed() || !m_plan)
		return;

	if (const CheckResult check = m_plan->Check(); !check) {
		Fail((*check.error()).what());
		return;
	}

	auto opened = m_plan->Source().m_origin->Visit([](auto&& held) {
		return FFmpeg::AVFormatContext::Open(held);
	});
	if (!opened.has_value()) {
		Fail(opened.error()->what());
		return;
	}

	auto engine = std::make_unique<Engine::Demux::Details::Container>();
	if (!engine->Adopt(*this, std::move(opened.value())))
		return;
	m_engine = std::move(engine);
	m_eof = false;
	m_positionNs.store(-1, std::memory_order_release);

	for (;;) {
		if (Failed())
			return;
		std::shared_ptr<Packet> packet = m_engine->Read(*this);
		if (Failed())
			return;
		if (!packet) {
			ReachedEof();
			return;
		}
		if (const auto& pts = packet->Pts(); pts)
			m_positionNs.store(pts->Nanoseconds().count(), std::memory_order_release);
		m_out->Push(packet);
	}
}

void Demux::Finish() noexcept {
	ReachedEof();
}

Decoder& StormByte::Multimedia::Pipeline::operator>>(Demux& demux, Decoder& decoder) noexcept {
	if (decoder.Failed())
		return decoder;
	static_cast<Step&>(demux) >> decoder;
	if (demux.Failed() || !demux.m_engine) {
		decoder.Fail(demux.Error().value_or("demuxer is not open"));
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

	if (demux.Plan()) {
		for (const auto& stream : demux.Plan()->Source().Streams()) {
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
	decoder.m_in->Notify(decoder.Wake());
	demux.m_out->Bind(decoder.Index(), *decoder.m_in);
	return decoder;
}

const StormByte::Multimedia::File& Demux::OriginFile() const noexcept {
	return m_plan->Source();
}
