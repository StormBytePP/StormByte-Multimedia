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
#include <StormByte/multimedia/backend/pipeline/detail/encoder/audio.hxx>
#include <StormByte/multimedia/backend/pipeline/frame.hxx>
#include <StormByte/multimedia/property/audio.hxx>
#include <StormByte/multimedia/property/channel_layout.hxx>
#include <StormByte/multimedia/type.hxx>

#include <utility>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/audio_fifo.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/rational.h>
	#include <libavutil/samplefmt.h>
	#include <libswresample/swresample.h>
}

using namespace StormByte::Multimedia;
using namespace StormByte::Multimedia::Backend::Pipeline::Detail::Encoder;

namespace {
	bool SameLayout(const AVChannelLayout& a, const AVChannelLayout& b) noexcept {
		return av_channel_layout_compare(&a, &b) == 0;
	}

	AVRational AudioTimeBase(const StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
		if (frame.Audio()) {
			const int rate = static_cast<int>(frame.Audio()->SampleRate());
			if (rate > 0)
				return AVRational{1, rate};
		}
		return AVRational{1, 48000};
	}

	StormByte::Multimedia::Backend::FFmpeg::AVCodecParameters FillAudioParams(
		const StormByte::Multimedia::Pipeline::Frame& frame,
		const std::optional<std::int64_t>& bitRate,
		const StormByte::Multimedia::Backend::FFmpeg::AVFrame* handle) noexcept {
		StormByte::Multimedia::Backend::FFmpeg::AVCodecParameters params(nullptr);
		if (bitRate)
			params.BitRate(*bitRate);
		if (frame.Audio()) {
			const auto& audio = *frame.Audio();
			params.SampleRate(static_cast<int>(audio.SampleRate()));
			if (!bitRate)
				params.BitRate(static_cast<std::int64_t>(audio.BitRate()));
			int channels = static_cast<int>(audio.Channels());
			if (channels <= 0)
				channels = static_cast<int>(Property::ChannelCount(audio.Layout()));
			params.DefaultChannelLayout(channels);
		}
		if (handle && handle->Get() && handle->Get()->ch_layout.nb_channels > 0 && params.Get())
			av_channel_layout_copy(&params.Get()->ch_layout, &handle->Get()->ch_layout);
		return params;
	}
}

Audio::Audio() noexcept
: m_owner(nullptr), m_timeBase{0, 1}, m_swr(nullptr), m_fifo(nullptr),
	m_inFormat(-1), m_outFormat(-1), m_frameSize(0), m_channels(0),
	m_index(0), m_nextPts(0), m_flushed(false), m_pktPts(0) {}

Audio::~Audio() noexcept {
	swr_free(&m_swr);
	if (m_fifo)
		av_audio_fifo_free(m_fifo);
}

Audio::Audio(Audio&& other) noexcept
: m_encoder(std::move(other.m_encoder)), m_scratch(std::move(other.m_scratch)),
	m_converted(std::move(other.m_converted)), m_pending(std::move(other.m_pending)),
	m_owner(other.m_owner), m_timeBase(other.m_timeBase), m_swr(other.m_swr), m_fifo(other.m_fifo),
	m_inFormat(other.m_inFormat), m_outFormat(other.m_outFormat),
	m_frameSize(other.m_frameSize), m_channels(other.m_channels),
	m_index(other.m_index), m_nextPts(other.m_nextPts), m_flushed(other.m_flushed),
	m_pktPts(other.m_pktPts) {
	other.m_swr = nullptr;
	other.m_fifo = nullptr;
	other.m_owner = nullptr;
}

Audio& Audio::operator=(Audio&& other) noexcept {
	if (this == &other)
		return *this;
	swr_free(&m_swr);
	if (m_fifo)
		av_audio_fifo_free(m_fifo);
	m_encoder = std::move(other.m_encoder);
	m_scratch = std::move(other.m_scratch);
	m_converted = std::move(other.m_converted);
	m_pending = std::move(other.m_pending);
	m_owner = other.m_owner;
	other.m_owner = nullptr;
	m_timeBase = other.m_timeBase;
	m_swr = other.m_swr;
	other.m_swr = nullptr;
	m_fifo = other.m_fifo;
	other.m_fifo = nullptr;
	m_inFormat = other.m_inFormat;
	m_outFormat = other.m_outFormat;
	m_frameSize = other.m_frameSize;
	m_channels = other.m_channels;
	m_index = other.m_index;
	m_nextPts = other.m_nextPts;
	m_flushed = other.m_flushed;
	m_pktPts = other.m_pktPts;
	return *this;
}

bool Audio::IsOpen() const noexcept {
	return m_encoder.has_value();
}

const AVCodecContext* Audio::Context() const noexcept {
	return m_encoder ? m_encoder->Get() : nullptr;
}

AVRational Audio::TimeBase() const noexcept {
	return m_timeBase;
}

bool Audio::PrepareConvert(StormByte::Multimedia::Pipeline::Encoder& owner,
	const ::AVFrame* src, const AVCodecContext* ctx) noexcept {
	if (!src || !ctx) {
		owner.Fail("audio convert missing source or encoder context");
		return false;
	}
	if (src->sample_rate != ctx->sample_rate) {
		owner.Fail("encoder sample rate does not match the decoded frame");
		return false;
	}
	m_inFormat = src->format;
	m_outFormat = ctx->sample_fmt;
	m_frameSize = ctx->frame_size > 0 ? ctx->frame_size : src->nb_samples;
	m_channels = ctx->ch_layout.nb_channels;
	if (m_frameSize <= 0 || m_channels <= 0) {
		owner.Fail("encoder audio frame size or channel count is invalid");
		return false;
	}

	const bool sameFmt = m_inFormat == m_outFormat;
	const bool sameLayout = SameLayout(src->ch_layout, ctx->ch_layout);
	if (!sameFmt || !sameLayout) {
		swr_free(&m_swr);
		int rc = swr_alloc_set_opts2(&m_swr,
			const_cast<AVChannelLayout*>(&ctx->ch_layout), static_cast<AVSampleFormat>(m_outFormat), ctx->sample_rate,
			const_cast<AVChannelLayout*>(&src->ch_layout), static_cast<AVSampleFormat>(m_inFormat), src->sample_rate,
			0, nullptr);
		if (rc < 0 || !m_swr) {
			owner.Fail("failed to allocate sample format converter");
			return false;
		}
		rc = swr_init(m_swr);
		if (rc < 0) {
			swr_free(&m_swr);
			owner.Fail("failed to init sample format converter");
			return false;
		}
	}

	if (m_fifo)
		av_audio_fifo_free(m_fifo);
	m_fifo = av_audio_fifo_alloc(static_cast<AVSampleFormat>(m_outFormat), m_channels, m_frameSize * 2);
	if (!m_fifo) {
		owner.Fail("failed to allocate audio fifo");
		return false;
	}
	m_nextPts = 0;
	return true;
}

bool Audio::Ingest(StormByte::Multimedia::Pipeline::Encoder& owner, ::AVFrame* src) noexcept {
	if (!src || !m_fifo || !m_encoder) {
		owner.Fail("audio ingest missing source or fifo");
		return false;
	}
	const auto* ctx = m_encoder->Get();
	if (!ctx) {
		owner.Fail("audio ingest missing encoder context");
		return false;
	}

	::AVFrame* ready = src;
	if (m_swr) {
		auto* dst = m_converted.Get();
		if (!dst) {
			owner.Fail("audio convert output frame is empty");
			return false;
		}
		av_frame_unref(dst);
		dst->format = m_outFormat;
		dst->sample_rate = ctx->sample_rate;
		dst->nb_samples = src->nb_samples;
		if (av_channel_layout_copy(&dst->ch_layout, &ctx->ch_layout) < 0) {
			owner.Fail("failed to copy audio channel layout");
			return false;
		}
		if (av_frame_get_buffer(dst, 0) < 0) {
			owner.Fail("failed to allocate converted audio buffer");
			return false;
		}
		if (swr_convert_frame(m_swr, dst, src) < 0) {
			owner.Fail("failed to convert audio sample format");
			return false;
		}
		ready = dst;
	}

	if (av_audio_fifo_realloc(m_fifo, av_audio_fifo_size(m_fifo) + ready->nb_samples) < 0) {
		owner.Fail("failed to grow audio fifo");
		return false;
	}
	if (av_audio_fifo_write(m_fifo, reinterpret_cast<void**>(ready->extended_data), ready->nb_samples) < ready->nb_samples) {
		owner.Fail("failed to write audio fifo");
		return false;
	}
	return true;
}

bool Audio::Emit(StormByte::Multimedia::Pipeline::Encoder& owner, bool last) noexcept {
	if (!m_encoder || !m_fifo)
		return true;
	const auto* ctx = m_encoder->Get();
	if (!ctx) {
		owner.Fail("audio emit missing encoder context");
		return false;
	}

	for (;;) {
		const int available = av_audio_fifo_size(m_fifo);
		int take = m_frameSize;
		if (available < m_frameSize) {
			if (!last || available <= 0)
				return true;
			take = available;
		}

		auto* dst = m_converted.Get();
		if (!dst) {
			owner.Fail("audio emit output frame is empty");
			return false;
		}
		av_frame_unref(dst);
		dst->format = m_outFormat;
		dst->sample_rate = ctx->sample_rate;
		dst->nb_samples = take;
		dst->pts = m_nextPts;
		dst->duration = take;
		if (av_channel_layout_copy(&dst->ch_layout, &ctx->ch_layout) < 0) {
			owner.Fail("failed to copy audio channel layout");
			return false;
		}
		if (av_frame_get_buffer(dst, 0) < 0) {
			owner.Fail("failed to allocate encoder audio buffer");
			return false;
		}
		if (av_audio_fifo_read(m_fifo, reinterpret_cast<void**>(dst->extended_data), take) < take) {
			owner.Fail("failed to read audio fifo");
			return false;
		}

		auto result = m_encoder->SendFrame(m_converted);
		while (result == StormByte::Multimedia::Backend::FFmpeg::OperationResult::TryAgain) {
			if (!DrainOne(owner)) {
				if (owner.Failed())
					return false;
				owner.Fail("encoder stalled");
				return false;
			}
			result = m_encoder->SendFrame(m_converted);
		}
		if (result == StormByte::Multimedia::Backend::FFmpeg::OperationResult::Error) {
			owner.Fail("failed to send frame");
			return false;
		}
		m_nextPts += take;
	}
}

bool Audio::Open(StormByte::Multimedia::Pipeline::Encoder& owner,
	const StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
	if (m_encoder)
		return true;
	if (frame.Type() != Type::Audio) {
		owner.Fail("encoder destination is audio but frame is not");
		return false;
	}
	const auto* handle = FrameHandle(owner, frame);
	if (!handle || !handle->Get()) {
		owner.Fail("frame has no backend buffer");
		return false;
	}

	auto opened = StormByte::Multimedia::Backend::Pipeline::Encoder::OpenCodec(
		owner, FillAudioParams(frame, owner.BitRate(), handle), AudioTimeBase(frame), owner.Require());
	if (!opened)
		return false;

	CommitOpen(owner, *opened);
	m_timeBase = opened->TimeBase();
	if (m_timeBase.num <= 0 || m_timeBase.den <= 0)
		m_timeBase = AudioTimeBase(frame);
	m_encoder = std::move(opened->Handle());
	m_index = owner.Index();
	m_owner = &owner;
	return PrepareConvert(owner, handle->Get(), m_encoder->Get());
}

bool Audio::Push(StormByte::Multimedia::Pipeline::Encoder& owner,
	const std::shared_ptr<StormByte::Multimedia::Pipeline::Frame>& frame) noexcept {
	if (!frame) {
		owner.Fail("empty frame");
		return false;
	}
	if (!m_encoder && !Open(owner, *frame))
		return false;
	if (!m_encoder)
		return false;
	auto* handle = FrameHandle(owner, *frame);
	if (!handle || !handle->Get()) {
		owner.Fail("frame has no backend buffer");
		return false;
	}

	handle->WriteSideData(frame->Attachments());

	auto* raw = handle->Get();
	if (frame->Pts())
		raw->pts = StormByte::Multimedia::Backend::Pipeline::Encoder::NsToTicks(
			frame->Pts()->Nanoseconds().count(), m_timeBase);
	else
		raw->pts = AV_NOPTS_VALUE;
	if (frame->Duration()) {
		raw->duration = StormByte::Multimedia::Backend::Pipeline::Encoder::NsToTicks(
			frame->Duration()->Nanoseconds().count(), m_timeBase);
		if (raw->duration <= 0)
			raw->duration = 1;
	}
	else
		raw->duration = 1;

	if (!Ingest(owner, raw))
		return false;
	if (!Emit(owner, false))
		return false;
	return true;
}

bool Audio::DrainOne(StormByte::Multimedia::Pipeline::Encoder& owner) noexcept {
	if (owner.Failed() || !m_encoder)
		return false;
	const auto result = m_encoder->ReceivePacket(m_scratch);
	if (result == StormByte::Multimedia::Backend::FFmpeg::OperationResult::TryAgain
		|| result == StormByte::Multimedia::Backend::FFmpeg::OperationResult::EndOfFile)
		return false;
	if (result != StormByte::Multimedia::Backend::FFmpeg::OperationResult::Success) {
		owner.Fail("failed to receive packet");
		return false;
	}
	StampOutgoing();
	m_pending.push_back(StormByte::Multimedia::Backend::Pipeline::Encoder::MakePacket(
		owner, Type::Audio, owner.Index(), m_scratch, m_timeBase, true));
	m_scratch.Unref();
	return true;
}

void Audio::Flush(StormByte::Multimedia::Pipeline::Encoder& owner) noexcept {
	if (owner.Failed() || !m_encoder || m_flushed)
		return;
	if (m_swr) {
		auto* dst = m_converted.Get();
		const auto* ctx = m_encoder->Get();
		if (dst && ctx) {
			av_frame_unref(dst);
			dst->format = m_outFormat;
			dst->sample_rate = ctx->sample_rate;
			dst->nb_samples = m_frameSize;
			if (av_channel_layout_copy(&dst->ch_layout, &ctx->ch_layout) == 0
				&& av_frame_get_buffer(dst, 0) == 0
				&& swr_convert_frame(m_swr, dst, nullptr) >= 0
				&& dst->nb_samples > 0
				&& m_fifo) {
				(void)av_audio_fifo_realloc(m_fifo, av_audio_fifo_size(m_fifo) + dst->nb_samples);
				(void)av_audio_fifo_write(m_fifo, reinterpret_cast<void**>(dst->extended_data), dst->nb_samples);
			}
		}
	}
	if (!Emit(owner, true))
		return;
	for (;;) {
		const auto sent = m_encoder->SetEof();
		if (sent == StormByte::Multimedia::Backend::FFmpeg::OperationResult::Success
			|| sent == StormByte::Multimedia::Backend::FFmpeg::OperationResult::EndOfFile)
			break;
		if (sent == StormByte::Multimedia::Backend::FFmpeg::OperationResult::TryAgain) {
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

std::shared_ptr<StormByte::Multimedia::Pipeline::Packet> Audio::Take() noexcept {
	if (m_pending.empty() && m_encoder && m_owner) {
		const auto result = m_encoder->ReceivePacket(m_scratch);
		if (result == StormByte::Multimedia::Backend::FFmpeg::OperationResult::Success) {
			StampOutgoing();
			m_pending.push_back(StormByte::Multimedia::Backend::Pipeline::Encoder::MakePacket(
				*m_owner, Type::Audio, m_index, m_scratch, m_timeBase, true));
			m_scratch.Unref();
		}
	}
	if (m_pending.empty())
		return {};
	auto packet = std::move(m_pending.front());
	m_pending.pop_front();
	return packet;
}

void Audio::StampOutgoing() noexcept {
	std::int64_t pts = m_scratch.Pts();
	std::int64_t dts = m_scratch.Dts();
	std::int64_t dur = m_scratch.Duration();

	if (dur <= 0)
		dur = m_frameSize > 0 ? m_frameSize : 1;

	if (pts == AV_NOPTS_VALUE)
		pts = m_pktPts;
	if (dts == AV_NOPTS_VALUE)
		dts = pts;
	if (pts < 0)
		pts = m_pktPts;
	if (dts < 0)
		dts = pts;

	m_pktPts = pts + dur;
	m_scratch.Timestamps(pts, dts, dur);
}
