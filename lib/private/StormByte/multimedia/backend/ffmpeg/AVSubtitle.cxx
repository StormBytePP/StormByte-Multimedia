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

#include <StormByte/multimedia/backend/ffmpeg/AVSubtitle.hxx>

extern "C" {
	#include <libavutil/avutil.h>
}

using namespace StormByte::Multimedia::Backend;

FFmpeg::AVSubtitle::AVSubtitle() noexcept
: m_sub{} {}

FFmpeg::AVSubtitle::AVSubtitle(AVSubtitle&& other) noexcept
: m_sub(other.m_sub) {
	other.m_sub = {};
}

FFmpeg::AVSubtitle::~AVSubtitle() noexcept {
	Free();
}

FFmpeg::AVSubtitle& FFmpeg::AVSubtitle::operator=(AVSubtitle&& other) noexcept {
	if (this != &other) {
		Free();
		m_sub = other.m_sub;
		other.m_sub = {};
	}
	return *this;
}

::AVSubtitle* FFmpeg::AVSubtitle::Get() noexcept {
	return &m_sub;
}

const ::AVSubtitle* FFmpeg::AVSubtitle::Get() const noexcept {
	return &m_sub;
}

std::int64_t FFmpeg::AVSubtitle::Pts() const noexcept {
	return m_sub.pts;
}

std::uint32_t FFmpeg::AVSubtitle::DisplayDurationMs() const noexcept {
	if (m_sub.end_display_time < m_sub.start_display_time)
		return 0;
	return m_sub.end_display_time - m_sub.start_display_time;
}

std::string FFmpeg::AVSubtitle::Text() const noexcept {
	std::string out;
	for (unsigned i = 0; i < m_sub.num_rects; ++i) {
		const AVSubtitleRect* rect = m_sub.rects[i];
		if (!rect)
			continue;
		const char* text = rect->text ? rect->text : rect->ass;
		if (!text)
			continue;
		if (!out.empty())
			out.push_back('\n');
		out.append(text);
	}
	return out;
}

void FFmpeg::AVSubtitle::Free() noexcept {
	avsubtitle_free(&m_sub);
	m_sub = {};
}
