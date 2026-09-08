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

#include <StormByte/multimedia/backend/ffmpeg/AVFrame.hxx>

#include <cstdint>
#include <cstring>

extern "C" {
	#include <libavutil/avutil.h>
	#include <libavutil/frame.h>
	#include <libavutil/hdr_dynamic_metadata.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/mastering_display_metadata.h>
	#include <libavutil/samplefmt.h>
}

using namespace StormByte::Multimedia::Backend;
using StormByte::Multimedia::Pipeline::SideDataKind;

namespace {
	constexpr int ChromaDenominator = 50000;
	constexpr int LumaDenominator = 10000;
}

FFmpeg::AVFrame::AVFrame() noexcept:
AVPointer(av_frame_alloc()) {}

FFmpeg::AVFrame::AVFrame(const AVFrame& other) noexcept:
AVPointer(other.m_ptr ? av_frame_clone(other.m_ptr) : av_frame_alloc()) {}

FFmpeg::AVFrame::~AVFrame() noexcept {
	Free();
}

FFmpeg::AVFrame& FFmpeg::AVFrame::operator=(const AVFrame& other) noexcept {
	if (this == &other)
		return *this;
	Free();
	m_ptr = other.m_ptr ? av_frame_clone(other.m_ptr) : av_frame_alloc();
	return *this;
}

void FFmpeg::AVFrame::Unref() noexcept {
	av_frame_unref(m_ptr);
}

const AVFrameSideData* FFmpeg::AVFrame::SideData(int type) const noexcept {
	if (!m_ptr)
		return nullptr;
	return av_frame_get_side_data(m_ptr, static_cast<AVFrameSideDataType>(type));
}

std::int64_t FFmpeg::AVFrame::Pts() const noexcept {
	return m_ptr ? m_ptr->pts : AV_NOPTS_VALUE;
}

std::int64_t FFmpeg::AVFrame::DurationTicks() const noexcept {
	return m_ptr ? m_ptr->duration : 0;
}

void FFmpeg::AVFrame::CopyPrimaryBuffer(StormByte::Buffer::DataType& out) const noexcept {
	out.clear();
	if (!m_ptr)
		return;

	if (m_ptr->width > 0 && m_ptr->height > 0 && m_ptr->data[0]) {
		const auto format = static_cast<AVPixelFormat>(m_ptr->format);
		const int size = av_image_get_buffer_size(format, m_ptr->width, m_ptr->height, 1);
		if (size <= 0)
			return;
		out.resize(static_cast<std::size_t>(size));
		if (av_image_copy_to_buffer(
			reinterpret_cast<std::uint8_t*>(out.data()), size,
			m_ptr->data, m_ptr->linesize,
			format, m_ptr->width, m_ptr->height, 1) < 0)
			out.clear();
		return;
	}

	if (m_ptr->nb_samples > 0 && m_ptr->data[0]) {
		const auto format = static_cast<AVSampleFormat>(m_ptr->format);
		const int bytes = av_samples_get_buffer_size(nullptr, m_ptr->ch_layout.nb_channels,
			m_ptr->nb_samples, format, 1);
		if (bytes <= 0)
			return;
		out.resize(static_cast<std::size_t>(bytes));
		auto* dst = reinterpret_cast<std::uint8_t*>(out.data());
		if (av_samples_copy(&dst, m_ptr->extended_data, 0, 0,
			m_ptr->nb_samples, m_ptr->ch_layout.nb_channels, format) < 0)
			out.clear();
	}
}

void FFmpeg::AVFrame::WriteHdr10(const StormByte::Multimedia::Property::HDR10& hdr10) noexcept {
	if (!m_ptr)
		return;

	const bool hasMastering =
		hdr10.Red().X() != 0 || hdr10.Red().Y() != 0 ||
		hdr10.Green().X() != 0 || hdr10.Green().Y() != 0 ||
		hdr10.Blue().X() != 0 || hdr10.Blue().Y() != 0 ||
		hdr10.White().X() != 0 || hdr10.White().Y() != 0 ||
		hdr10.Luminance().X() != 0 || hdr10.Luminance().Y() != 0;
	const auto& light = hdr10.LightLevel();
	const bool hasLight = light.has_value() && (light->X() != 0 || light->Y() != 0);

	if (hasMastering) {
		av_frame_remove_side_data(m_ptr, AV_FRAME_DATA_MASTERING_DISPLAY_METADATA);
		auto* mdm = av_mastering_display_metadata_create_side_data(m_ptr);
		if (mdm) {
			mdm->has_primaries = 1;
			mdm->display_primaries[0][0] = AVRational{hdr10.Red().X(), ChromaDenominator};
			mdm->display_primaries[0][1] = AVRational{hdr10.Red().Y(), ChromaDenominator};
			mdm->display_primaries[1][0] = AVRational{hdr10.Green().X(), ChromaDenominator};
			mdm->display_primaries[1][1] = AVRational{hdr10.Green().Y(), ChromaDenominator};
			mdm->display_primaries[2][0] = AVRational{hdr10.Blue().X(), ChromaDenominator};
			mdm->display_primaries[2][1] = AVRational{hdr10.Blue().Y(), ChromaDenominator};
			mdm->white_point[0] = AVRational{hdr10.White().X(), ChromaDenominator};
			mdm->white_point[1] = AVRational{hdr10.White().Y(), ChromaDenominator};
			mdm->has_luminance = 1;
			mdm->min_luminance = AVRational{hdr10.Luminance().X(), LumaDenominator};
			mdm->max_luminance = AVRational{hdr10.Luminance().Y(), LumaDenominator};
		}
	}

	if (hasLight) {
		av_frame_remove_side_data(m_ptr, AV_FRAME_DATA_CONTENT_LIGHT_LEVEL);
		auto* cll = av_content_light_metadata_create_side_data(m_ptr);
		if (cll) {
			cll->MaxCLL = static_cast<unsigned>(light->X());
			cll->MaxFALL = static_cast<unsigned>(light->Y());
		}
	}
}

void FFmpeg::AVFrame::WriteSideData(
	const std::vector<StormByte::Multimedia::Pipeline::SideData>& attachments) noexcept {
	if (!m_ptr)
		return;

	for (const auto& item : attachments) {
		const auto size = item.Payload().AvailableBytes();
		if (size == 0)
			continue;
		StormByte::Buffer::DataType bytes;
		if (!item.Payload().Peek(size, bytes) || bytes.empty())
			continue;

		if (item.Kind() == SideDataKind::MasteringDisplay
			|| item.Kind() == SideDataKind::ContentLight)
			continue;

		if (item.Kind() == SideDataKind::HdrPlus) {
			av_frame_remove_side_data(m_ptr, AV_FRAME_DATA_DYNAMIC_HDR_PLUS);
			AVDynamicHDRPlus* plus = av_dynamic_hdr_plus_create_side_data(m_ptr);
			if (!plus)
				continue;
			const std::size_t copy = std::min(bytes.size(), sizeof(AVDynamicHDRPlus));
			std::memcpy(plus, bytes.data(), copy);
			continue;
		}

		AVFrameSideDataType type = AV_FRAME_DATA_SEI_UNREGISTERED;
		switch (item.Kind()) {
			case SideDataKind::A53CC:
				type = AV_FRAME_DATA_A53_CC;
				break;
			default:
				type = AV_FRAME_DATA_SEI_UNREGISTERED;
				break;
		}
		AVFrameSideData* side = av_frame_new_side_data(m_ptr, type, static_cast<int>(bytes.size()));
		if (!side)
			continue;
		std::memcpy(side->data, bytes.data(), bytes.size());
	}
}

void FFmpeg::AVFrame::Free() noexcept {
	if (m_ptr) {
		av_frame_free(&m_ptr);
		m_ptr = nullptr;
	}
}

template class StormByte::Multimedia::Backend::FFmpeg::AVPointer<::AVFrame>;
