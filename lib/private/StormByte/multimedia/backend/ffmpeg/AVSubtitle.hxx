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

#pragma once

#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <string>

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace StormByte::Multimedia::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Backend::FFmpeg {
	/**
	 * @class AVSubtitle
	 * @brief RAII wrapper for ::AVSubtitle (`avsubtitle_free`).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVSubtitle {
		public:
			/**
			 * @brief Empty subtitle.
			 */
			AVSubtitle() noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			AVSubtitle(const AVSubtitle&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source subtitle.
			 */
			AVSubtitle(AVSubtitle&& other) noexcept;

			/**
			 * @brief Destructor.
			 */
			~AVSubtitle() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			AVSubtitle& operator=(const AVSubtitle&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source subtitle.
			 * @return *this.
			 */
			AVSubtitle& operator=(AVSubtitle&& other) noexcept;

			/**
			 * @brief Mutable FFmpeg struct.
			 * @return Pointer to the owned struct.
			 */
			::AVSubtitle* Get() noexcept;

			/**
			 * @brief Const FFmpeg struct.
			 * @return Pointer to the owned struct.
			 */
			const ::AVSubtitle* Get() const noexcept;

			/**
			 * @brief Presentation timestamp in `AV_TIME_BASE`.
			 * @return PTS, or `AV_NOPTS_VALUE`.
			 */
			std::int64_t Pts() const noexcept;

			/**
			 * @brief Display duration in milliseconds.
			 * @return `end_display_time - start_display_time`, or 0.
			 */
			std::uint32_t DisplayDurationMs() const noexcept;

			/**
			 * @brief Concatenated text / ASS from rects.
			 * @return UTF-8 text, or empty.
			 */
			std::string Text() const noexcept;

			/**
			 * @brief Releases rects (`avsubtitle_free`).
			 */
			void Free() noexcept;

		private:
			::AVSubtitle m_sub;	///< Owned subtitle
	};
}
