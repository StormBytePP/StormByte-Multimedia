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

#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/details/audio.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/details/subtitle.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/details/video.hxx>
#include <StormByte/multimedia/type.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/rational.h>
	#include <libavutil/samplefmt.h>
}

using namespace StormByte::Multimedia::Pipeline;

Encoder::Encoder(int output_index, const StormByte::Multimedia::Codec& codec) noexcept
: m_index(output_index), m_codec(&codec),
m_encoderTag("StormByte-Multimedia " STORMBYTE_MULTIMEDIA_VERSION),
m_failed(false) {
	switch (codec.Type()) {
		case StormByte::Multimedia::Type::Video:
			m_engine = std::make_unique<Engine::Encoder::Details::Video>();
			break;
		case StormByte::Multimedia::Type::Audio:
			m_engine = std::make_unique<Engine::Encoder::Details::Audio>();
			break;
		case StormByte::Multimedia::Type::Subtitle:
			m_engine = std::make_unique<Engine::Encoder::Details::Subtitle>();
			break;
		default:
			Fail("encoder destination type is not video, audio or subtitle");
			break;
	}
}

Encoder::Encoder(Encoder&&) noexcept = default;
Encoder::~Encoder() noexcept = default;
Encoder& Encoder::operator=(Encoder&&) noexcept = default;

Encoder::operator bool() const noexcept {
	return !m_failed && m_engine && m_engine->IsOpen();
}

int Encoder::Index() const noexcept {
	return m_index;
}

const StormByte::Multimedia::Codec& Encoder::Destination() const noexcept {
	return *m_codec;
}

bool Encoder::Failed() const noexcept {
	return m_failed;
}

const std::optional<std::string>& Encoder::Error() const noexcept {
	return m_error;
}

bool Encoder::Opened() const noexcept {
	return !m_failed && m_engine && m_engine->IsOpen();
}

std::optional<int> Encoder::AudioChannels() const noexcept {
	const auto* ctx = (m_engine && m_engine->IsOpen()) ? m_engine->Context() : nullptr;
	if (!ctx || ctx->ch_layout.nb_channels <= 0)
		return std::nullopt;
	return ctx->ch_layout.nb_channels;
}

std::optional<int> Encoder::AudioSampleRate() const noexcept {
	const auto* ctx = (m_engine && m_engine->IsOpen()) ? m_engine->Context() : nullptr;
	if (!ctx || ctx->sample_rate <= 0)
		return std::nullopt;
	return ctx->sample_rate;
}

std::optional<int> Encoder::AudioFrameSize() const noexcept {
	const auto* ctx = (m_engine && m_engine->IsOpen()) ? m_engine->Context() : nullptr;
	if (!ctx || ctx->frame_size <= 0)
		return std::nullopt;
	return ctx->frame_size;
}

std::optional<int> Encoder::AudioSampleFormat() const noexcept {
	const auto* ctx = (m_engine && m_engine->IsOpen()) ? m_engine->Context() : nullptr;
	if (!ctx || ctx->sample_fmt == AV_SAMPLE_FMT_NONE)
		return std::nullopt;
	return static_cast<int>(ctx->sample_fmt);
}

const std::optional<std::string>& Encoder::Language() const noexcept {
	return m_language;
}

void Encoder::Language(std::string language) noexcept {
	if (language.empty())
		m_language.reset();
	else
		m_language = std::move(language);
}

const std::optional<std::string>& Encoder::Title() const noexcept {
	return m_title;
}

void Encoder::Title(std::string title) noexcept {
	if (title.empty())
		m_title.reset();
	else
		m_title = std::move(title);
}

const std::string& Encoder::EncoderTag() const noexcept {
	return m_encoderTag;
}

const std::optional<std::string>& Encoder::Implementation() const noexcept {
	return m_implementation;
}

void Encoder::Implementation(std::string name) noexcept {
	if (name.empty())
		m_implementation.reset();
	else
		m_implementation = std::move(name);
}

const StormByte::Multimedia::Features& Encoder::Require() const noexcept {
	return m_require;
}

void Encoder::Require(StormByte::Multimedia::Features features) noexcept {
	m_require = features;
}

const StormByte::Multimedia::Features& Encoder::Capabilities() const noexcept {
	return m_capabilities;
}

void Encoder::CRF(int value) noexcept {
	if (m_failed)
		return;
	if (m_codec->Type() != StormByte::Multimedia::Type::Video) {
		Fail("CRF is not valid for this codec");
		return;
	}
	if (m_bitRate.has_value()) {
		Fail("CRF cannot be combined with BitRate");
		return;
	}
	m_crf = value;
}

const std::optional<int>& Encoder::CRF() const noexcept {
	return m_crf;
}

void Encoder::BitRate(std::int64_t bits_per_second) noexcept {
	if (m_failed)
		return;
	if (bits_per_second <= 0) {
		Fail("BitRate must be positive");
		return;
	}
	if (m_crf.has_value()) {
		Fail("BitRate cannot be combined with CRF");
		return;
	}
	if (m_maxBitRate.has_value() && bits_per_second > *m_maxBitRate) {
		Fail("BitRate exceeds MaxBitRate");
		return;
	}
	m_bitRate = bits_per_second;
}

const std::optional<std::int64_t>& Encoder::BitRate() const noexcept {
	return m_bitRate;
}

void Encoder::MaxBitRate(std::int64_t bits_per_second) noexcept {
	if (m_failed)
		return;
	if (m_codec->Type() != StormByte::Multimedia::Type::Video) {
		Fail("MaxBitRate is not valid for this codec");
		return;
	}
	if (bits_per_second <= 0) {
		Fail("MaxBitRate must be positive");
		return;
	}
	if (m_bitRate.has_value() && *m_bitRate > bits_per_second) {
		Fail("BitRate exceeds MaxBitRate");
		return;
	}
	m_maxBitRate = bits_per_second;
}

const std::optional<std::int64_t>& Encoder::MaxBitRate() const noexcept {
	return m_maxBitRate;
}

void Encoder::Preset(std::string name) noexcept {
	if (m_failed)
		return;
	if (name.empty()) {
		m_preset.reset();
		return;
	}
	m_preset = std::move(name);
}

const std::optional<std::string>& Encoder::Preset() const noexcept {
	return m_preset;
}

void Encoder::Tune(std::string name) noexcept {
	if (m_failed)
		return;
	if (m_codec->Type() != StormByte::Multimedia::Type::Video) {
		Fail("Tune is not valid for this codec");
		return;
	}
	if (name.empty()) {
		m_tune.reset();
		return;
	}
	m_tune = std::move(name);
}

const std::optional<std::string>& Encoder::Tune() const noexcept {
	return m_tune;
}

const std::map<std::string, std::string>& Encoder::FineTune() const noexcept {
	return m_fineTune;
}

void Encoder::FineTune(std::map<std::string, std::string> options) noexcept {
	m_fineTune = std::move(options);
}

void Encoder::Flush() noexcept {
	if (m_failed || !m_engine)
		return;
	m_engine->Flush(*this);
}

void Encoder::Fail(std::string reason) noexcept {
	m_failed = true;
	m_error = std::move(reason);
	m_capabilities = StormByte::Multimedia::Features{};
	m_engine.reset();
}

bool Encoder::MuxBindStream(void* avStream) noexcept {
	if (!m_engine || !avStream)
		return false;
	auto* stream = static_cast<AVStream*>(avStream);
	const auto* ctx = m_engine->Context();
	if (!ctx)
		return false;
	if (avcodec_parameters_from_context(stream->codecpar, ctx) < 0)
		return false;
	AVRational tb = m_engine->TimeBase();
	if (tb.num <= 0 || tb.den <= 0)
		tb = ctx->time_base;
	if (tb.num <= 0 || tb.den <= 0)
		tb = AVRational{1, 1000};
	stream->time_base = tb;
	return true;
}

bool Encoder::MuxTakePacket(Packet& packet) noexcept {
	if (!m_engine)
		return false;
	if (m_engine->TakePacket(packet))
		return true;
	if (!m_engine->DrainOne(*this))
		return false;
	return m_engine->TakePacket(packet);
}

Frame& StormByte::Multimedia::Pipeline::operator>>(Frame& frame, Encoder& encoder) noexcept {
	if (encoder.m_failed || !encoder.m_engine)
		return frame;
	if (frame.Language() && !encoder.m_language)
		encoder.Language(*frame.Language());
	if (frame.Title() && !encoder.m_title)
		encoder.Title(*frame.Title());
	(void)encoder.m_engine->Push(encoder, frame);
	return frame;
}

Encoder& StormByte::Multimedia::Pipeline::operator>>(Encoder& encoder, Packet& packet) noexcept {
	if (encoder.m_failed || !encoder.m_engine)
		return encoder;
	if (encoder.m_engine->TakePacket(packet))
		return encoder;
	if (!encoder.m_engine->DrainOne(encoder))
		return encoder;
	(void)encoder.m_engine->TakePacket(packet);
	return encoder;
}
