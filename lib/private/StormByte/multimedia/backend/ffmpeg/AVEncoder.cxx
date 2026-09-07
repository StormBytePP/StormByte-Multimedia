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
#include <StormByte/multimedia/backend/ffmpeg/AVEncoder.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVPacket.hxx>

extern "C" {
	#include <libavutil/opt.h>
}

using namespace StormByte::Multimedia::Backend;

FFmpeg::AVEncoder::AVEncoder(::AVCodecContext* ctx) noexcept
:AVPointer(ctx) {}

FFmpeg::AVEncoder::~AVEncoder() noexcept {
	Free();
}

FFmpeg::ExpectedAVEncoder FFmpeg::AVEncoder::Open(AVCodec* codec, const AVCodecParameters& params, int stream_index,
	const std::map<std::string, std::string>& options, AVRational time_base) noexcept {
	if (!codec || !params.Get())
		return Unexpected<FFmpeg::EncoderError>("Invalid codec or parameters");

	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
		return Unexpected<FFmpeg::EncoderError>("Out of memory allocating codec context");

	if (avcodec_parameters_to_context(ctx, params.Get()) < 0) {
		avcodec_free_context(&ctx);
		return Unexpected<FFmpeg::EncoderError>("Failed to copy codec parameters to encoder context");
	}

	ctx->thread_count = 0;
	if (time_base.num > 0 && time_base.den > 0) {
		ctx->time_base = time_base;
		ctx->pkt_timebase = time_base;
	}

	for (const auto& [key, value] : options) {
		if (key.empty())
			continue;
		if (av_opt_set(ctx, key.c_str(), value.c_str(), AV_OPT_SEARCH_CHILDREN) < 0) {
			avcodec_free_context(&ctx);
			return Unexpected<FFmpeg::EncoderError>("failed to set encoder option '" + key + "'");
		}
	}

	if (avcodec_open2(ctx, codec, nullptr) < 0) {
		avcodec_free_context(&ctx);
		return Unexpected<FFmpeg::EncoderError>("Failed to open encoder");
	}

	AVEncoder enc(ctx);
	enc.m_stream_index = stream_index;
	return enc;
}

FFmpeg::ExpectedAVEncoder FFmpeg::AVEncoder::Open(AVCodec* codec, const AVCodecParameters& params, const FFmpeg::AVFormatContext& fmt, int stream_index) noexcept {
	auto opened = Open(codec, params, stream_index, {}, AVRational{0, 1});
	if (!opened.has_value())
		return opened;
	auto bsf = fmt.Mp4ToAnnexB(params.Get()->codec_id, stream_index, params);
	if (bsf)
		opened->m_bsf_pipeline.Add(std::move(*bsf));
	return opened;
}

FFmpeg::OperationResult FFmpeg::AVEncoder::SendFrame(AVFrame& frame) noexcept {
	int ret = avcodec_send_frame(m_ptr, frame.Get());
	switch (ret) {
		case 0:
			return OperationResult::Success;
		case AVERROR(EAGAIN):
			return OperationResult::TryAgain;
		case AVERROR_EOF:
			return OperationResult::EndOfFile;
		default:
			return OperationResult::Error;
	}
}

FFmpeg::OperationResult FFmpeg::AVEncoder::ReceivePacket(AVPacket& pkt) noexcept {
	AVPacket tmp;
	int ret = avcodec_receive_packet(m_ptr, tmp.Get());
	if (ret == AVERROR(EAGAIN))
		return OperationResult::TryAgain;
	if (ret == AVERROR_EOF)
		return OperationResult::EndOfFile;
	if (ret < 0)
		return OperationResult::Error;

	const auto filtered = m_bsf_pipeline.Process(tmp);
	if (filtered != OperationResult::Success)
		return filtered;

	pkt = std::move(tmp);
	return OperationResult::Success;
}

int FFmpeg::AVEncoder::StreamIndex() const noexcept {
	return m_stream_index;
}

AVRational FFmpeg::AVEncoder::TimeBase() const noexcept {
	if (!m_ptr)
		return AVRational{0, 1};
	return m_ptr->time_base;
}

void FFmpeg::AVEncoder::Flush() noexcept {
	avcodec_flush_buffers(m_ptr);
	m_bsf_pipeline.Flush();
}

void FFmpeg::AVEncoder::SetEof() noexcept {
	avcodec_send_frame(m_ptr, nullptr);
	m_bsf_pipeline.SetEof();
}

void FFmpeg::AVEncoder::Free() noexcept {
	if (m_ptr) {
		avcodec_free_context(&m_ptr);
		m_ptr = nullptr;
	}
}

template class StormByte::Multimedia::Backend::FFmpeg::AVPointer<::AVCodecContext>;
