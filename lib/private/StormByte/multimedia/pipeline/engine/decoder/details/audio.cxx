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

#include <StormByte/multimedia/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/details/audio.hxx>
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>

#include <cstdint>

extern "C" {
	#include <libavutil/avutil.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/rational.h>
}

namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;
namespace Details = StormByte::Multimedia::Pipeline::Engine::Decoder::Details;

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

	std::int64_t NsToTicks(const std::optional<StormByte::Multimedia::Property::Duration>& value, AVRational timeBase) noexcept {
		if (!value.has_value() || timeBase.num <= 0 || timeBase.den <= 0)
			return AV_NOPTS_VALUE;
		return av_rescale_q(value->Nanoseconds().count(), AVRational{1, 1000000000}, timeBase);
	}

	void StampTags(class StormByte::Multimedia::Pipeline::Decoder& decoder,
		class StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
		if (decoder.Language())
			frame.Language(*decoder.Language());
		if (decoder.Title())
			frame.Title(*decoder.Title());
	}
}

Details::Audio::Audio(FFmpeg::AVDecoder decoder, AVRational timeBase,
	std::optional<StormByte::Multimedia::Property::Audio> audio) noexcept
: m_decoder(std::move(decoder)), m_audio(std::move(audio)), m_timeBase(timeBase) {}

bool Details::Audio::IsOpen() const noexcept {
	return true;
}

bool Details::Audio::Send(class Decoder& owner, Packet& packet) noexcept {
	FFmpeg::AVPacket raw;
	StormByte::Buffer::DataType bytes;
	const auto n = packet.Payload().AvailableBytes();
	const std::uint8_t* data = nullptr;
	if (n > 0) {
		if (!packet.Payload().Extract(n, bytes) || bytes.size() != n) {
			owner.Fail("failed to extract packet payload");
			return false;
		}
		data = reinterpret_cast<const std::uint8_t*>(bytes.data());
	}
	if (!raw.Load(data, static_cast<int>(n), owner.Index(), packet.KeyFrame())) {
		owner.Fail("out of memory copying packet");
		return false;
	}

	const std::int64_t duration = packet.Duration()
		? av_rescale_q(packet.Duration()->Nanoseconds().count(), AVRational{1, 1000000000}, m_timeBase)
		: 0;
	raw.Timestamps(NsToTicks(packet.Pts(), m_timeBase), NsToTicks(packet.Dts(), m_timeBase), duration);

	auto result = m_decoder.SendPacket(raw);
	while (result == FFmpeg::OperationResult::TryAgain) {
		StormByte::Multimedia::Pipeline::Frame ignored;
		if (!Receive(owner, ignored))
			break;
		if (owner.Failed())
			return false;
		result = m_decoder.SendPacket(raw);
	}
	if (result == FFmpeg::OperationResult::Error) {
		owner.Fail("failed to send packet");
		return false;
	}
	return true;
}

bool Details::Audio::Receive(class Decoder& owner, class Frame& frame) noexcept {
	auto holder = std::make_unique<StormByte::Multimedia::Pipeline::Engine::Frame::Engine>();
	const auto result = m_decoder.ReceiveFrame(holder->m_backend);
	if (result == FFmpeg::OperationResult::TryAgain || result == FFmpeg::OperationResult::EndOfFile)
		return false;
	if (result != FFmpeg::OperationResult::Success) {
		owner.Fail("failed to receive frame");
		return false;
	}

	frame = StormByte::Multimedia::Pipeline::Frame(
		Multimedia::Type::Audio,
		owner.Index(),
		StormByte::Buffer::FIFO{},
		TicksToPts(holder->m_backend.Pts(), m_timeBase),
		TicksToDuration(holder->m_backend.DurationTicks(), m_timeBase),
		std::nullopt,
		{},
		m_audio
	);
	StampTags(owner, frame);
	frame.Bind(std::move(holder));
	return true;
}

void Details::Audio::Flush(class Decoder& owner) noexcept {
	if (owner.Failed() || m_flushed)
		return;
	for (;;) {
		const auto sent = m_decoder.SetEof();
		if (sent == FFmpeg::OperationResult::Success || sent == FFmpeg::OperationResult::EndOfFile)
			break;
		if (sent == FFmpeg::OperationResult::TryAgain)
			break;
		owner.Fail("failed to signal decoder EOF");
		return;
	}
	m_flushed = true;
}
