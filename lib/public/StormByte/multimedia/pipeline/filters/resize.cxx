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

#include <StormByte/multimedia/pipeline/filters/resize.hxx>
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>
#include <StormByte/multimedia/property/video.hxx>
#include <StormByte/multimedia/type.hxx>

extern "C" {
	#include <libavutil/frame.h>
	#include <libavutil/imgutils.h>
	#include <libswscale/swscale.h>
}

using namespace StormByte::Multimedia::Pipeline::Filter;
using StormByte::Multimedia::Type;

Resize::Resize(const StormByte::Multimedia::Property::Resolution& resolution) noexcept
: m_width(resolution.Width()), m_height(resolution.Height()) {}

Resize::Resize(std::uint32_t width, std::uint32_t height) noexcept
: Resize(StormByte::Multimedia::Property::Resolution{
	width == 0 ? 1u : width,
	height == 0 ? 1u : height}) {
	m_width = width;
	m_height = height;
}

std::optional<StormByte::Multimedia::Pipeline::Frame> Resize::Push(
	StormByte::Multimedia::Pipeline::Frame&& frame) noexcept {
	if (Failed())
		return std::nullopt;
	if (frame.Type() != Type::Video || !frame.m_engine)
		return std::move(frame);

	::AVFrame* src = frame.m_engine->m_backend.Get();
	if (!src || src->width <= 0 || src->height <= 0) {
		Fail("resize: missing video buffer");
		return std::nullopt;
	}

	if (m_width == 0 && m_height == 0) {
		Fail("resize: width and height are both 0");
		return std::nullopt;
	}

	std::uint32_t dstW = m_width;
	std::uint32_t dstH = m_height;
	if (dstW == 0)
		dstW = static_cast<std::uint32_t>(
			(static_cast<std::uint64_t>(src->width) * dstH + src->height / 2) / src->height);
	if (dstH == 0)
		dstH = static_cast<std::uint32_t>(
			(static_cast<std::uint64_t>(src->height) * dstW + src->width / 2) / src->width);
	if (dstW == 0 || dstH == 0) {
		Fail("resize: computed destination is empty");
		return std::nullopt;
	}

	if (static_cast<int>(dstW) == src->width && static_cast<int>(dstH) == src->height)
		return std::move(frame);

	StormByte::Multimedia::Backend::FFmpeg::AVFrame dst;
	::AVFrame* out = dst.Get();
	if (!out) {
		Fail("resize: out of memory");
		return std::nullopt;
	}
	if (av_frame_copy_props(out, src) < 0) {
		Fail("resize: failed to copy frame properties");
		return std::nullopt;
	}
	out->width = static_cast<int>(dstW);
	out->height = static_cast<int>(dstH);
	out->format = src->format;
	if (av_frame_get_buffer(out, 0) < 0) {
		Fail("resize: failed to allocate destination");
		return std::nullopt;
	}

	SwsContext* sws = sws_getContext(
		src->width, src->height, static_cast<AVPixelFormat>(src->format),
		out->width, out->height, static_cast<AVPixelFormat>(out->format),
		SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!sws) {
		Fail("resize: swscale rejected this format");
		return std::nullopt;
	}
	const int scaled = sws_scale(sws, src->data, src->linesize, 0, src->height,
		out->data, out->linesize);
	sws_freeContext(sws);
	if (scaled <= 0) {
		Fail("resize: swscale failed");
		return std::nullopt;
	}

	frame.m_engine->m_backend = std::move(dst);
	frame.m_engine->m_payloadReady = false;
	frame.m_payload = StormByte::Buffer::FIFO{};
	frame.m_video = StormByte::Multimedia::Property::Video(
		frame.Video()->Color(),
		StormByte::Multimedia::Property::Resolution{dstW, dstH},
		frame.Video()->HDR10());
	return std::move(frame);
}
