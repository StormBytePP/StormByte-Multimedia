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

extern "C" {
	#include <libavutil/avutil.h>
}

using namespace StormByte::Multimedia::Backend;

FFmpeg::AVFrame::AVFrame() noexcept:
AVPointer(av_frame_alloc()) {}

FFmpeg::AVFrame::~AVFrame() noexcept {
	Free();
}

void FFmpeg::AVFrame::Unref() noexcept {
	av_frame_unref(m_ptr);
}

const AVFrameSideData* FFmpeg::AVFrame::SideData(int type) const noexcept {
	if (!m_ptr)
		return nullptr;
	return av_frame_get_side_data(m_ptr, static_cast<AVFrameSideDataType>(type));
}

void FFmpeg::AVFrame::CopyPrimaryBuffer(StormByte::Buffer::DataType& out) const noexcept {
	out.clear();
	if (!m_ptr || !m_ptr->buf[0] || m_ptr->buf[0]->size <= 0)
		return;
	const auto* p = reinterpret_cast<const std::byte*>(m_ptr->buf[0]->data);
	out.assign(p, p + m_ptr->buf[0]->size);
}

std::int64_t FFmpeg::AVFrame::Pts() const noexcept {
	if (!m_ptr)
		return AV_NOPTS_VALUE;
	return m_ptr->pts != AV_NOPTS_VALUE ? m_ptr->pts : m_ptr->best_effort_timestamp;
}

std::int64_t FFmpeg::AVFrame::DurationTicks() const noexcept {
	return m_ptr ? m_ptr->duration : 0;
}

void FFmpeg::AVFrame::Free() noexcept {
	if (m_ptr) {
		av_frame_free(&m_ptr);
		m_ptr = nullptr;
	}
}

template class StormByte::Multimedia::Backend::FFmpeg::AVPointer<::AVFrame>;
