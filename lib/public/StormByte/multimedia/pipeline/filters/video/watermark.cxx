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

#include <StormByte/multimedia/pipeline/filters/video/watermark.hxx>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <utility>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/frame.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/pixfmt.h>
	#include <libswscale/swscale.h>
}

using namespace StormByte::Multimedia::Pipeline::Filter::Video;

namespace {
	const AVCodec* CodecFromPath(const std::filesystem::path& path) noexcept {
		std::string ext = path.extension().string();
		for (char& c : ext)
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		if (ext == ".png")
			return avcodec_find_decoder(AV_CODEC_ID_PNG);
		if (ext == ".jpg" || ext == ".jpeg")
			return avcodec_find_decoder(AV_CODEC_ID_MJPEG);
		if (ext == ".webp")
			return avcodec_find_decoder(AV_CODEC_ID_WEBP);
		if (ext == ".bmp")
			return avcodec_find_decoder(AV_CODEC_ID_BMP);
		return avcodec_find_decoder(AV_CODEC_ID_MJPEG);
	}

	std::pair<int, int> Place(int frameW, int frameH, int logoW, int logoH,
		const std::optional<Anchor>& anchor,
		const std::optional<StormByte::Multimedia::Property::Point>& point,
		int margin) noexcept {
		if (point.has_value())
			return {point->X(), point->Y()};
		const Anchor a = anchor.value_or(Anchor::BottomRight);
		int x = margin;
		int y = margin;
		switch (a) {
			case Anchor::TopLeft:		x = margin; y = margin; break;
			case Anchor::TopCenter:		x = (frameW - logoW) / 2; y = margin; break;
			case Anchor::TopRight:		x = frameW - logoW - margin; y = margin; break;
			case Anchor::CenterLeft:	x = margin; y = (frameH - logoH) / 2; break;
			case Anchor::Center:		x = (frameW - logoW) / 2; y = (frameH - logoH) / 2; break;
			case Anchor::CenterRight:	x = frameW - logoW - margin; y = (frameH - logoH) / 2; break;
			case Anchor::BottomLeft:	x = margin; y = frameH - logoH - margin; break;
			case Anchor::BottomCenter:	x = (frameW - logoW) / 2; y = frameH - logoH - margin; break;
			case Anchor::BottomRight:	x = frameW - logoW - margin; y = frameH - logoH - margin; break;
		}
		return {x, y};
	}

	void Blend(uint8_t* dst, int dstLinesize, int frameW, int frameH,
		const uint8_t* logo, int logoW, int logoH, int x, int y, unsigned opacity) noexcept {
		const int alphaScale = static_cast<int>(opacity);
		for (int row = 0; row < logoH; ++row) {
			const int fy = y + row;
			if (fy < 0 || fy >= frameH)
				continue;
			uint8_t* line = dst + fy * dstLinesize;
			const uint8_t* src = logo + row * logoW * 4;
			for (int col = 0; col < logoW; ++col) {
				const int fx = x + col;
				if (fx < 0 || fx >= frameW)
					continue;
				const uint8_t* px = src + col * 4;
				const int a = (static_cast<int>(px[3]) * alphaScale) / 100;
				if (a <= 0)
					continue;
				uint8_t* d = line + fx * 4;
				const int ia = 255 - a;
				d[0] = static_cast<uint8_t>((d[0] * ia + px[0] * a) / 255);
				d[1] = static_cast<uint8_t>((d[1] * ia + px[1] * a) / 255);
				d[2] = static_cast<uint8_t>((d[2] * ia + px[2] * a) / 255);
				d[3] = 255;
			}
		}
	}
}

Watermark::Watermark(const std::filesystem::path& logo, Anchor anchor,
	unsigned opacity, int margin) noexcept
: m_path(logo), m_anchor(anchor),
m_opacity(std::min(opacity, 100u)), m_margin(margin) {}

Watermark::Watermark(const std::filesystem::path& logo,
	StormByte::Multimedia::Property::Point position, unsigned opacity) noexcept
: m_path(logo), m_point(position),
m_opacity(std::min(opacity, 100u)), m_margin(0) {}

StormByte::Multimedia::Type Watermark::Media() const noexcept {
	return StormByte::Multimedia::Type::Video;
}

bool Watermark::LoadFile() noexcept {
	if (m_loaded)
		return !m_bytes.empty();
	m_loaded = true;

	std::ifstream in(m_path, std::ios::binary);
	if (!in) {
		Fail("watermark: cannot open " + m_path.string());
		return false;
	}
	in.seekg(0, std::ios::end);
	const auto size = in.tellg();
	if (size <= 0) {
		Fail("watermark: empty logo");
		return false;
	}
	in.seekg(0, std::ios::beg);
	m_bytes.resize(static_cast<std::size_t>(size));
	in.read(reinterpret_cast<char*>(m_bytes.data()), size);
	if (!in) {
		m_bytes.clear();
		Fail("watermark: failed to read " + m_path.string());
		return false;
	}
	return true;
}

bool Watermark::DecodeLogo() noexcept {
	if (m_decoded)
		return !m_rgba.empty();
	m_decoded = true;
	if (!LoadFile())
		return false;

	const AVCodec* codec = CodecFromPath(m_path);
	if (!codec) {
		Fail("watermark: no decoder for logo");
		return false;
	}

	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx || avcodec_open2(ctx, codec, nullptr) < 0) {
		avcodec_free_context(&ctx);
		Fail("watermark: failed to open logo decoder");
		return false;
	}

	AVPacket* pkt = av_packet_alloc();
	AVFrame* decoded = av_frame_alloc();
	if (!pkt || !decoded) {
		av_packet_free(&pkt);
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		Fail("watermark: out of memory");
		return false;
	}

	pkt->data = reinterpret_cast<uint8_t*>(m_bytes.data());
	pkt->size = static_cast<int>(m_bytes.size());
	bool ok = avcodec_send_packet(ctx, pkt) >= 0 && avcodec_receive_frame(ctx, decoded) >= 0;
	pkt->data = nullptr;
	pkt->size = 0;
	av_packet_free(&pkt);

	if (!ok || decoded->width <= 0 || decoded->height <= 0) {
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		Fail("watermark: failed to decode logo");
		return false;
	}

	AVFrame* rgba = av_frame_alloc();
	if (!rgba) {
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		Fail("watermark: out of memory");
		return false;
	}
	rgba->format = AV_PIX_FMT_RGBA;
	rgba->width = decoded->width;
	rgba->height = decoded->height;
	if (av_frame_get_buffer(rgba, 0) < 0) {
		av_frame_free(&rgba);
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		Fail("watermark: failed to allocate RGBA logo");
		return false;
	}

	SwsContext* sws = sws_getContext(
		decoded->width, decoded->height, static_cast<AVPixelFormat>(decoded->format),
		rgba->width, rgba->height, AV_PIX_FMT_RGBA,
		SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!sws || sws_scale(sws, decoded->data, decoded->linesize, 0, decoded->height,
			rgba->data, rgba->linesize) <= 0) {
		if (sws)
			sws_freeContext(sws);
		av_frame_free(&rgba);
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		Fail("watermark: failed to convert logo to RGBA");
		return false;
	}
	sws_freeContext(sws);

	m_logoWidth = rgba->width;
	m_logoHeight = rgba->height;
	m_rgba.resize(static_cast<std::size_t>(m_logoWidth) * static_cast<std::size_t>(m_logoHeight) * 4);
	for (int row = 0; row < m_logoHeight; ++row) {
		const uint8_t* src = rgba->data[0] + row * rgba->linesize[0];
		uint8_t* dst = reinterpret_cast<uint8_t*>(m_rgba.data()) +
			static_cast<std::size_t>(row) * static_cast<std::size_t>(m_logoWidth) * 4;
		std::copy(src, src + m_logoWidth * 4, dst);
	}

	av_frame_free(&rgba);
	av_frame_free(&decoded);
	avcodec_free_context(&ctx);
	return true;
}

void Watermark::ProcessFrame(Pipeline::Frame& frame) noexcept {
	if (m_opacity == 0)
		return;
	if (!DecodeLogo())
		return;

	::AVFrame* src = Native(frame);
	if (!src || src->width <= 0 || src->height <= 0) {
		Fail("watermark: missing video buffer");
		return;
	}

	const auto [x, y] = Place(src->width, src->height, m_logoWidth, m_logoHeight,
		m_anchor, m_point, m_margin);
	if (x < 0 || y < 0 || x + m_logoWidth > src->width || y + m_logoHeight > src->height) {
		Fail("watermark: logo does not fit in the frame");
		return;
	}

	::AVFrame* out = av_frame_alloc();
	if (!out) {
		Fail("watermark: out of memory");
		return;
	}
	if (av_frame_copy_props(out, src) < 0) {
		av_frame_free(&out);
		Fail("watermark: failed to copy frame properties");
		return;
	}
	out->width = src->width;
	out->height = src->height;
	out->format = AV_PIX_FMT_RGBA;
	if (av_frame_get_buffer(out, 0) < 0) {
		av_frame_free(&out);
		Fail("watermark: failed to allocate destination");
		return;
	}

	SwsContext* toRgba = sws_getContext(
		src->width, src->height, static_cast<AVPixelFormat>(src->format),
		out->width, out->height, AV_PIX_FMT_RGBA,
		SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!toRgba || sws_scale(toRgba, src->data, src->linesize, 0, src->height,
			out->data, out->linesize) <= 0) {
		if (toRgba)
			sws_freeContext(toRgba);
		av_frame_free(&out);
		Fail("watermark: failed to convert frame to RGBA");
		return;
	}
	sws_freeContext(toRgba);

	Blend(out->data[0], out->linesize[0], out->width, out->height,
		reinterpret_cast<const uint8_t*>(m_rgba.data()),
		m_logoWidth, m_logoHeight, x, y, m_opacity);

	if (src->format != AV_PIX_FMT_RGBA) {
		::AVFrame* restored = av_frame_alloc();
		if (!restored) {
			av_frame_free(&out);
			Fail("watermark: out of memory");
			return;
		}
		if (av_frame_copy_props(restored, src) < 0) {
			av_frame_free(&restored);
			av_frame_free(&out);
			Fail("watermark: failed to copy frame properties");
			return;
		}
		restored->width = src->width;
		restored->height = src->height;
		restored->format = src->format;
		if (av_frame_get_buffer(restored, 0) < 0) {
			av_frame_free(&restored);
			av_frame_free(&out);
			Fail("watermark: failed to allocate destination");
			return;
		}
		SwsContext* fromRgba = sws_getContext(
			out->width, out->height, AV_PIX_FMT_RGBA,
			restored->width, restored->height, static_cast<AVPixelFormat>(restored->format),
			SWS_BILINEAR, nullptr, nullptr, nullptr);
		if (!fromRgba || sws_scale(fromRgba, out->data, out->linesize, 0, out->height,
				restored->data, restored->linesize) <= 0) {
			if (fromRgba)
				sws_freeContext(fromRgba);
			av_frame_free(&restored);
			av_frame_free(&out);
			Fail("watermark: failed to convert frame back");
			return;
		}
		sws_freeContext(fromRgba);
		av_frame_free(&out);
		out = restored;
	}

	Replace(frame, out);
}
