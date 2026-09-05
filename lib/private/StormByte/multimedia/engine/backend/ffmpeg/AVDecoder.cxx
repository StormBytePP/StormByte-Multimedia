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

#include <StormByte/multimedia/engine/backend/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVDecoder.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVPacket.hxx>

using namespace StormByte::Multimedia::Engine::Backend;

FFmpeg::AVDecoder::AVDecoder(::AVCodecContext* ctx) noexcept:
AVPointer(ctx) {}

FFmpeg::AVDecoder::~AVDecoder() noexcept {
	Free();
}

FFmpeg::ExpectedAVDecoder FFmpeg::AVDecoder::Open(AVCodec* codec, const AVCodecParameters& params, const AVFormatContext& fmt, int stream_index) noexcept {
	if (!codec || !params.Get())
		return Unexpected<DecoderError>("Invalid codec or parameters");

	// Allocate context
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
		return Unexpected<DecoderError>("Out of memory allocating codec context");

	// Copy parameters
	if (avcodec_parameters_to_context(ctx, params.Get()) < 0) {
		avcodec_free_context(&ctx);
		return Unexpected<DecoderError>("Failed to copy codec parameters");
	}

	// Open decoder
	if (avcodec_open2(ctx, codec, nullptr) < 0) {
		avcodec_free_context(&ctx);
		return Unexpected<DecoderError>("Failed to open decoder");
	}

	// Construct decoder RAII wrapper
	AVDecoder dec(ctx);
	dec.m_stream_index = stream_index;

	// Checks for required BSF
	auto bsf = fmt.Mp4ToAnnexB(params.Get()->codec_id, stream_index, params);
	if (bsf)
		dec.m_bsf_pipeline.Add(std::move(*bsf));

	return dec;
}

FFmpeg::OperationResult FFmpeg::AVDecoder::SendPacket(AVPacket& pkt) noexcept {
	// Ensure packet belongs to the decoder's stream
	if (pkt.StreamIndex() != m_stream_index)
		return OperationResult::Error;

	AVPacket filtered_pkt = pkt.Ref();
	// Send to BSF pipeline
	int ret = m_bsf_pipeline.Process(filtered_pkt);
	if (ret < 0) {
		return OperationResult::Error;
	}

	// Now send filtered packet to decoder
	ret = avcodec_send_packet(m_ptr, filtered_pkt.Get());

	if (ret == AVERROR(EAGAIN))
		return OperationResult::TryAgain;

	if (ret == AVERROR_EOF)
		return OperationResult::EndOfFile;

	if (ret < 0)
		return OperationResult::Error;

	return OperationResult::Success;
}

FFmpeg::OperationResult FFmpeg::AVDecoder::ReceiveFrame(AVFrame& frame) noexcept {
	int ret = avcodec_receive_frame(m_ptr, frame.Get());
	switch(ret) {
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

int FFmpeg::AVDecoder::StreamIndex() const noexcept {
	return m_stream_index;
}

void FFmpeg::AVDecoder::Flush() noexcept {
    avcodec_flush_buffers(m_ptr);
    m_bsf_pipeline.Flush();
}

void FFmpeg::AVDecoder::SetEof() noexcept {
	avcodec_send_packet(m_ptr, nullptr);
	m_bsf_pipeline.SetEof();
}

void FFmpeg::AVDecoder::Free() noexcept {
	if (m_ptr) {
		avcodec_free_context(&m_ptr);
		m_ptr = nullptr;
	}
}

// Explicit template instantiation
template class StormByte::Multimedia::Engine::Backend::FFmpeg::AVPointer<::AVCodecContext>;