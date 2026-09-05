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

#include <StormByte/multimedia/property/color.hxx>
#include <StormByte/multimedia/property/hdr10.hxx>
#include <StormByte/multimedia/property/resolution.hxx>
#include <StormByte/multimedia/visibility.h>

#include <optional>

/**
 * @namespace StormByte::Multimedia::Property
 * @brief Media property value types.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @class Video
	 * @brief Per-stream video properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Video final {
		public:
			/**
			 * @brief Constructs video properties.
			 * @param color Colorimetry and pixel format.
			 * @param resolution Frame size.
			 * @param hdr10 Optional mastering-display metadata.
			 */
			Video(Color color, Resolution resolution, std::optional<HDR10> hdr10 = std::nullopt) noexcept;

			/**
			 * @brief Copy constructor.
			 */
			Video(const Video&) = default;

			/**
			 * @brief Move constructor.
			 */
			Video(Video&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Video() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @return *this.
			 */
			Video& operator=(const Video&) = default;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Video& operator=(Video&&) noexcept = default;

			/**
			 * @brief Colorimetry and pixel format.
			 * @return Color.
			 */
			const class Color& Color() const noexcept;

			/**
			 * @brief Frame size.
			 * @return Resolution.
			 */
			const class Resolution& Resolution() const noexcept;

			/**
			 * @brief Mastering-display metadata, if present.
			 * @return HDR10, or empty.
			 */
			const std::optional<class HDR10>& HDR10() const noexcept;

		private:
			class Color m_color;						///< Color
			class Resolution m_resolution;			///< Frame size
			std::optional<class HDR10> m_hdr10;		///< Optional HDR10
	};
}
