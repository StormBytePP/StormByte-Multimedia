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

#include <StormByte/multimedia/pipeline/filters/watermark.hxx>
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>
#include <StormByte/multimedia/type.hxx>

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/frame.h>
	#include <libswscale/swscale.h>
}

using namespace StormByte::Multimedia::Pipeline::Filter;
using StormByte::Multimedia::Type;

class Watermark::Impl {
	public:
		int width = 0;
		int height = 0;
		StormByte::Buffer::DataType rgba;
};

namespace {
	const AVCodec* CodecFromMime(const std::optional<std::string>& mime,
		const std::optional<std::string>& name) noexcept {
		const std::string key = mime.value_or("") + " " + name.value_or("");
		if (key.find("png") != std::string::npos)
			return avcodec_find_decoder(AV_CODEC_ID_PNG);
		if (key.find("jpeg") != std::string::npos || key.find("jpg") != std::string::npos)
			return avcodec_find_decoder(AV_CODEC_ID_MJPEG);
		if (key.find("webp") != std::string::npos)
			return avcodec_find_decoder(AV_CODEC_ID_WEBP);
		if (key.find("bmp") != std::string::npos)
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
}

Watermark::Watermark(const StormByte::Multimedia::Attachment& logo, Anchor anchor,
	unsigned opacity, int margin) noexcept
: m_mime(logo.MimeType()), m_name(logo.FileName()), m_anchor(anchor),
m_opacity(std::min(opacity, 100u)), m_margin(margin) {
	logo.Payload().Peek(logo.Payload().AvailableBytes(), m_bytes);
}

Watermark::Watermark(const StormByte::Multimedia::Attachment& logo,
	StormByte::Multimedia::Property::Point position, unsigned opacity) noexcept
: m_mime(logo.MimeType()), m_name(logo.FileName()), m_point(position),
m_opacity(std::min(opacity, 100u)), m_margin(0) {
	logo.Payload().Peek(logo.Payload().AvailableBytes(), m_bytes);
}

Watermark::Watermark(Watermark&&) noexcept = default;
Watermark::~Watermark() noexcept = default;
Watermark& Watermark::operator=(Watermark&&) noexcept = default;

std::optional<StormByte::Multimedia::Pipeline::Frame> Watermark::Push(
	StormByte::Multimedia::Pipeline::Frame&& frame) noexcept {
	if (Failed())
		return std::nullopt;
	if (m_opacity == 0)
		return std::move(frame);
	if (frame.Type() != Type::Video || !frame.m_engine)
		return std::move(frame);

	::AVFrame* dst = frame.m_engine->m_backend.Get();
	if (!dst || dst->width <= 0 || dst->height <= 0) {
		Fail("watermark: missing video buffer");
		return std::nullopt;
	}

	if (!m_impl) {
		if (m_bytes.empty()) {
			Fail("watermark: empty logo");
			return std::nullopt;
		}
		const AVCodec* codec = CodecFromMime(m_mime, m_name);
		if (!codec) {
			Fail("watermark: no decoder for logo");
			return std::nullopt;
		}
		AVCodecContext* ctx = avcodec_alloc_context3(codec);
		if (!ctx || avcodec_open2(ctx, codec, nullptr) < 0) {
			avcodec_free_context(&ctx);
			Fail("watermark: failed to open logo decoder");
			return std::nullopt;
		}
		AVPacket* pkt = av_packet_alloc();
		AVFrame* raw = av_frame_alloc();
		if (!pkt || !raw) {
			av_packet_free(&pkt);
			av_frame_free(&raw);
			avcodec_free_context(&ctx);
			Fail("watermark: out of memory");
			return std::nullopt;
		}
		pkt->data = reinterpret_cast<std::uint8_t*>(m_bytes.data());
		pkt->size = static_cast<int>(m_bytes.size());
		if (avcodec_send_packet(ctx, pkt) < 0 || avcodec_receive_frame(ctx, raw) < 0) {
			av_packet_free(&pkt);
			av_frame_free(&raw);
			avcodec_free_context(&ctx);
			Fail("watermark: failed to decode logo");
			return std::nullopt;
		}

		auto impl = std::make_unique<Impl>();
		impl->width = raw->width;
		impl->height = raw->height;
		impl->rgba.resize(static_cast<std::size_t>(raw->width) * static_cast<std::size_t>(raw->height) * 4);
		SwsContext* toRgba = sws_getContext(raw->width, raw->height, static_cast<AVPixelFormat>(raw->format),
			raw->width, raw->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
		if (!toRgba) {
			av_packet_free(&pkt);
			av_frame_free(&raw);
			avcodec_free_context(&ctx);
			Fail("watermark: cannot convert logo");
			return std::nullopt;
		}
		std::uint8_t* planes[4]{reinterpret_cast<std::uint8_t*>(impl->rgba.data()), nullptr, nullptr, nullptr};
		int linesize[4]{raw->width * 4, 0, 0, 0};
		sws_scale(toRgba, raw->data, raw->linesize, 0, raw->height, planes, linesize);
		sws_freeContext(toRgba);
		av_packet_free(&pkt);
		av_frame_free(&raw);
		avcodec_free_context(&ctx);
		m_impl = std::move(impl);
	}

	const auto [x0, y0] = Place(dst->width, dst->height, m_impl->width, m_impl->height,
		m_anchor, m_point, m_margin);
	if (x0 < 0 || y0 < 0 || x0 + m_impl->width > dst->width || y0 + m_impl->height > dst->height) {
		Fail("watermark: logo does not fit");
		return std::nullopt;
	}

	const int rgbaSize = dst->width * dst->height * 4;
	std::vector<std::uint8_t> canvas(static_cast<std::size_t>(rgbaSize));
	SwsContext* toRgba = sws_getContext(dst->width, dst->height, static_cast<AVPixelFormat>(dst->format),
		dst->width, dst->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);
	SwsContext* fromRgba = sws_getContext(dst->width, dst->height, AV_PIX_FMT_RGBA,
		dst->width, dst->height, static_cast<AVPixelFormat>(dst->format), SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!toRgba || !fromRgba) {
		if (toRgba)
			sws_freeContext(toRgba);
		if (fromRgba)
			sws_freeContext(fromRgba);
		Fail("watermark: swscale rejected this format");
		return std::nullopt;
	}
	std::uint8_t* dstPlanes[4]{canvas.data(), nullptr, nullptr, nullptr};
	int dstLinesize[4]{dst->width * 4, 0, 0, 0};
	sws_scale(toRgba, dst->data, dst->linesize, 0, dst->height, dstPlanes, dstLinesize);
	sws_freeContext(toRgba);

	const unsigned op = m_opacity;
	for (int y = 0; y < m_impl->height; ++y) {
		for (int x = 0; x < m_impl->width; ++x) {
			const std::size_t li = (static_cast<std::size_t>(y) * m_impl->width + x) * 4;
			const std::size_t fi = (static_cast<std::size_t>(y0 + y) * dst->width + (x0 + x)) * 4;
			const unsigned sa = (static_cast<unsigned>(m_impl->rgba[li + 3]) * op) / 100;
			const unsigned da = 255 - sa;
			for (int c = 0; c < 3; ++c)
				canvas[fi + c] = static_cast<std::uint8_t>(
					(static_cast<unsigned>(m_impl->rgba[li + c]) * sa +
					static_cast<unsigned>(canvas[fi + c]) * da) / 255);
		}
	}

	const std::uint8_t* srcPlanes[4]{canvas.data(), nullptr, nullptr, nullptr};
	int srcLinesize[4]{dst->width * 4, 0, 0, 0};
	sws_scale(fromRgba, srcPlanes, srcLinesize, 0, dst->height, dst->data, dst->linesize);
	sws_freeContext(fromRgba);

	frame.m_engine->m_payloadReady = false;
	frame.m_payload = StormByte::Buffer::FIFO{};
	return std::move(frame);
}
