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
#include <StormByte/multimedia/backend/ffmpeg/AVSubtitle.hxx>

#include <cstring>
#include <string>

extern "C" {
	#include <libavutil/error.h>
	#include <libavutil/mem.h>
	#include <libavutil/opt.h>
}

using namespace StormByte::Multimedia::Backend;

namespace {
	constexpr const char DefaultAssHeader[] =
		"[Script Info]\n"
		"ScriptType: v4.00+\n"
		"\n"
		"[V4+ Styles]\n"
		"Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, OutlineColour, BackColour, Bold, Italic, Underline, StrikeOut, ScaleX, ScaleY, Spacing, Angle, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, Encoding\n"
		"Style: Default,Arial,16,&Hffffff,&Hffffff,&H0,&H0,0,0,0,0,100,100,0,0,1,1,0,2,10,10,10,0\n"
		"\n"
		"[Events]\n"
		"Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n";

	std::string AvError(int err) noexcept {
		char buf[AV_ERROR_MAX_STRING_SIZE];
		av_strerror(err, buf, sizeof(buf));
		return buf;
	}
}

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

	if (ctx->codec_type == AVMEDIA_TYPE_SUBTITLE && !ctx->subtitle_header) {
		const auto n = std::strlen(DefaultAssHeader);
		ctx->subtitle_header = static_cast<std::uint8_t*>(av_malloc(n + 1));
		if (!ctx->subtitle_header) {
			avcodec_free_context(&ctx);
			return Unexpected<FFmpeg::EncoderError>("Out of memory allocating subtitle header");
		}
		std::memcpy(ctx->subtitle_header, DefaultAssHeader, n + 1);
		ctx->subtitle_header_size = static_cast<int>(n);
	}

	for (const auto& [key, value] : options) {
		if (key.empty())
			continue;
		if (av_opt_set(ctx, key.c_str(), value.c_str(), AV_OPT_SEARCH_CHILDREN) < 0) {
			avcodec_free_context(&ctx);
			return Unexpected<FFmpeg::EncoderError>("failed to set encoder option '" + key + "'");
		}
	}

	const int opened = avcodec_open2(ctx, codec, nullptr);
	if (opened < 0) {
		const std::string why = AvError(opened);
		avcodec_free_context(&ctx);
		return Unexpected<FFmpeg::EncoderError>("Failed to open encoder: " + why);
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

FFmpeg::OperationResult FFmpeg::AVEncoder::EncodeSubtitle(AVSubtitle& sub, AVPacket& pkt) noexcept {
	if (!m_ptr || m_ptr->codec_type != AVMEDIA_TYPE_SUBTITLE)
		return OperationResult::Error;

	std::uint8_t buf[65536];
	const int n = avcodec_encode_subtitle(m_ptr, buf, static_cast<int>(sizeof(buf)), sub.Get());
	if (n < 0)
		return OperationResult::Error;
	if (n == 0)
		return OperationResult::TryAgain;
	if (!pkt.Load(buf, n, m_stream_index, false))
		return OperationResult::Error;

	const std::int64_t pts = sub.Pts();
	const AVRational tb = (m_ptr->time_base.num > 0) ? m_ptr->time_base : AVRational{1, AV_TIME_BASE};
	const std::int64_t duration = av_rescale_q(static_cast<std::int64_t>(sub.DisplayDurationMs()),
		AVRational{1, 1000}, tb);
	const std::int64_t scaled = (pts == AV_NOPTS_VALUE)
		? AV_NOPTS_VALUE
		: av_rescale_q(pts, AVRational{1, AV_TIME_BASE}, tb);
	pkt.Timestamps(scaled, scaled, duration);
	return OperationResult::Success;
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

bool FFmpeg::AVEncoder::IsSubtitle() const noexcept {
	return m_ptr && m_ptr->codec_type == AVMEDIA_TYPE_SUBTITLE;
}

void FFmpeg::AVEncoder::Flush() noexcept {
	avcodec_flush_buffers(m_ptr);
	m_bsf_pipeline.Flush();
}

void FFmpeg::AVEncoder::SetEof() noexcept {
	if (m_ptr && m_ptr->codec_type != AVMEDIA_TYPE_SUBTITLE)
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
