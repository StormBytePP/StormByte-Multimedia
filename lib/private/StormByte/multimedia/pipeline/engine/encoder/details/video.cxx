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

#include <StormByte/multimedia/pipeline/engine/encoder/details/video.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/open.hxx>
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>
#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/type.hxx>

#include <cstdint>
#include <cstdio>
#include <memory>
#include <utility>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/rational.h>
}

namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;
namespace Open = StormByte::Multimedia::Pipeline::Engine::Encoder::Open;
using StormByte::Multimedia::Type;

StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::Video() noexcept
: m_timeBase{0, 1}, m_index(0), m_flushed(false), m_tsOffset(0), m_tsOffsetSet(false) {}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::IsOpen() const noexcept {
	return m_encoder.has_value();
}

const AVCodecContext* StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::Context() const noexcept {
	return m_encoder ? m_encoder->Get() : nullptr;
}

AVRational StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::TimeBase() const noexcept {
	return m_timeBase;
}

void StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::StampOutgoing() noexcept {
	std::int64_t pts = m_scratch.Pts();
	std::int64_t dts = m_scratch.Dts();
	std::int64_t dur = m_scratch.Duration();

	if (!m_tsOffsetSet) {
		/* Presentation origin: first PTS → 0. DTS may go negative (B delay). */
		m_tsOffset = (pts != AV_NOPTS_VALUE) ? -pts : 0;
		m_tsOffsetSet = true;
	}

	if (pts != AV_NOPTS_VALUE)
		pts += m_tsOffset;
	if (dts != AV_NOPTS_VALUE)
		dts += m_tsOffset;

	if (dts == AV_NOPTS_VALUE)
		dts = (pts != AV_NOPTS_VALUE) ? pts : 0;
	if (pts == AV_NOPTS_VALUE)
		pts = dts;
	/* Do not clamp pts up to dts: B-frames are PTS ahead of a later P,
	but the first I is PTS >= DTS after this shift (PTS=0, DTS=-delay). */

	if (dur <= 0)
		dur = 1;
	m_scratch.Timestamps(pts, dts, dur);
}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::Open(
	class StormByte::Multimedia::Pipeline::Encoder& owner,
	const class StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
	if (m_encoder)
		return true;
	if (frame.Type() != Type::Video) {
		owner.Fail("encoder destination is video but frame is not");
		return false;
	}
	auto opened = Open::OpenBackend(owner, frame);
	if (!opened)
		return false;
	m_timeBase = opened->timeBase;
	if (m_timeBase.num <= 0 || m_timeBase.den <= 0)
		m_timeBase = AVRational{1, 24};
	if (!opened->implementation.empty())
		owner.Implementation(opened->implementation);
	owner.m_capabilities = opened->capabilities;
	m_encoder = std::move(opened->encoder);
	m_index = owner.Index();
	return true;
}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::Push(
	class StormByte::Multimedia::Pipeline::Encoder& owner,
	const std::shared_ptr<StormByte::Multimedia::Pipeline::Frame>& frame) noexcept {
	if (!frame) {
		owner.Fail("empty frame");
		return false;
	}
	if (!m_encoder && !Open(owner, *frame))
		return false;
	if (!m_encoder)
		return false;
	if (!frame->m_engine || !frame->m_engine->m_backend.Get()) {
		owner.Fail("frame has no backend buffer");
		return false;
	}

	if (frame->Video() && frame->Video()->HDR10())
		frame->m_engine->m_backend.WriteHdr10(*frame->Video()->HDR10());
	frame->m_engine->m_backend.WriteSideData(frame->Attachments());

	auto* raw = frame->m_engine->m_backend.Get();
	if (frame->Pts())
		raw->pts = Open::NsToTicks(frame->Pts()->Nanoseconds().count(), m_timeBase);
	else
		raw->pts = AV_NOPTS_VALUE;
	if (frame->Duration()) {
		raw->duration = Open::NsToTicks(frame->Duration()->Nanoseconds().count(), m_timeBase);
		if (raw->duration <= 0)
			raw->duration = 1;
	}
	else
		raw->duration = 1;

	const auto result = m_encoder->SendFrame(frame->m_engine->m_backend);
	if (result == FFmpeg::OperationResult::TryAgain)
		return false;
	if (result == FFmpeg::OperationResult::Error) {
		owner.Fail("failed to send frame");
		return false;
	}
	return true;
}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::DrainOne(
	class StormByte::Multimedia::Pipeline::Encoder& owner) noexcept {
	if (owner.Failed() || !m_encoder)
		return false;
	const auto result = m_encoder->ReceivePacket(m_scratch);
	if (result == FFmpeg::OperationResult::TryAgain || result == FFmpeg::OperationResult::EndOfFile)
		return false;
	if (result != FFmpeg::OperationResult::Success) {
		owner.Fail("failed to receive packet");
		return false;
	}
	StampOutgoing();
	const auto* ctx = m_encoder->Get();
	const bool keepPacketHdrPlus = !ctx || ctx->codec_id != AV_CODEC_ID_HEVC;
	m_pending.push_back(Open::MakePacket(Type::Video, owner.Index(), m_scratch, m_timeBase, keepPacketHdrPlus));
	m_scratch.Unref();
	return true;
}

void StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::Flush(
	class StormByte::Multimedia::Pipeline::Encoder& owner) noexcept {
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

std::shared_ptr<StormByte::Multimedia::Pipeline::Packet>
StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Video::Take() noexcept {
	if (m_pending.empty() && m_encoder) {
		const auto result = m_encoder->ReceivePacket(m_scratch);
		if (result == FFmpeg::OperationResult::Success) {
			StampOutgoing();
			const auto* ctx = m_encoder->Get();
			const bool keepPacketHdrPlus = !ctx || ctx->codec_id != AV_CODEC_ID_HEVC;
			m_pending.push_back(Open::MakePacket(Type::Video, m_index, m_scratch, m_timeBase, keepPacketHdrPlus));
			m_scratch.Unref();
		}
	}
	if (m_pending.empty())
		return {};
	auto packet = std::move(m_pending.front());
	m_pending.pop_front();
	return packet;
}
