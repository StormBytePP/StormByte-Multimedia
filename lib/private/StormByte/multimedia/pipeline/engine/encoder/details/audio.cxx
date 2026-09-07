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

#include <StormByte/multimedia/pipeline/engine/encoder/open.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/details/audio.hxx>
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/rational.h>
}

namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;
namespace Open = StormByte::Multimedia::Pipeline::Engine::Encoder::Open;

StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Audio::Audio() noexcept = default;

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Audio::IsOpen() const noexcept {
	return m_encoder.has_value();
}

const AVCodecContext* StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Audio::Context() const noexcept {
	return m_encoder ? m_encoder->Get() : nullptr;
}

AVRational StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Audio::TimeBase() const noexcept {
	return m_timeBase;
}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Audio::Open(class Encoder& owner, const class Frame& frame) noexcept {
	if (m_encoder)
		return true;
	if (!frame.Audio()) {
		owner.Fail("encoder destination is audio but frame is not");
		return false;
	}
	auto opened = Open::OpenBackend(owner, frame);
	if (!opened)
		return false;
	m_timeBase = opened->timeBase;
	if (m_timeBase.num <= 0 || m_timeBase.den <= 0) {
		const int rate = static_cast<int>(frame.Audio()->SampleRate());
		m_timeBase = rate > 0 ? AVRational{1, rate} : AVRational{1, 48000};
	}
	if (!opened->implementation.empty())
		owner.Implementation(opened->implementation);
	owner.m_capabilities = opened->capabilities;
	m_encoder = std::move(opened->encoder);
	return true;
}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Audio::Push(class Encoder& owner, class Frame& frame) noexcept {
	if (!m_encoder && !Open(owner, frame))
		return false;
	if (!m_encoder)
		return false;
	if (!frame.m_engine || !frame.m_engine->m_backend.Get()) {
		owner.Fail("frame has no backend buffer");
		return false;
	}

	frame.m_engine->m_backend.WriteSideData(frame.Attachments());

	auto* raw = frame.m_engine->m_backend.Get();
	if (frame.Pts())
		raw->pts = Open::NsToTicks(frame.Pts()->Nanoseconds().count(), m_timeBase);
	else
		raw->pts = AV_NOPTS_VALUE;
	if (frame.Duration()) {
		raw->duration = Open::NsToTicks(frame.Duration()->Nanoseconds().count(), m_timeBase);
		if (raw->duration <= 0)
			raw->duration = 1;
	}
	else
		raw->duration = 1;

	auto result = m_encoder->SendFrame(frame.m_engine->m_backend);
	while (result == FFmpeg::OperationResult::TryAgain) {
		if (!DrainOne(owner)) {
			if (owner.Failed())
				return false;
			owner.Fail("encoder stalled");
			return false;
		}
		result = m_encoder->SendFrame(frame.m_engine->m_backend);
	}
	if (result == FFmpeg::OperationResult::Error) {
		owner.Fail("failed to send frame");
		return false;
	}
	return true;
}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Audio::DrainOne(class Encoder& owner) noexcept {
	if (owner.Failed() || !m_encoder)
		return false;
	const auto result = m_encoder->ReceivePacket(m_scratch);
	if (result == FFmpeg::OperationResult::TryAgain || result == FFmpeg::OperationResult::EndOfFile)
		return false;
	if (result != FFmpeg::OperationResult::Success) {
		owner.Fail("failed to receive packet");
		return false;
	}
	m_pending.push_back(Open::MakePacket(owner.Index(), m_scratch, m_timeBase, true));
	m_scratch.Unref();
	return true;
}

void StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Audio::Flush(class Encoder& owner) noexcept {
	if (owner.Failed() || !m_encoder || m_flushed)
		return;
	for (;;) {
		const auto sent = m_encoder->SetEof();
		if (sent == FFmpeg::OperationResult::Success || sent == FFmpeg::OperationResult::EndOfFile)
			break;
		if (sent == FFmpeg::OperationResult::TryAgain) {
			if (!DrainOne(owner))
				break;
			continue;
		}
		owner.Fail("failed to signal encoder EOF");
		return;
	}
	while (DrainOne(owner))
		;
	m_flushed = true;
}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Audio::TakePacket(Packet& packet) noexcept {
	if (!m_pending.empty()) {
		packet = std::move(m_pending.front());
		m_pending.pop_front();
		return true;
	}
	return false;
}
