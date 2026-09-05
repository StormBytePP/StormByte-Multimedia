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

#include <StormByte/multimedia/context/generic.hxx>
#include <StormByte/multimedia/context/property/color.hxx>
#include <StormByte/multimedia/context/property/hdr10.hxx>
#include <StormByte/multimedia/context/property/resolution.hxx>

#include <optional>

/**
 * @namespace Context
 * @brief Media stream context types (audio, video, …).
 */
namespace StormByte::Multimedia::Context {
	/**
	 * @class Video
	 * @brief Video stream context (color, resolution, optional HDR10).
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Video final: public Generic {
		public:
			/**
			 * @param color Color properties.
			 * @param resolution Frame size.
			 * @param hdr10 Optional HDR10 metadata (filled with DEFAULT if color allows HDR10 and none given).
			 */
			Video(Property::Color&& color, Property::Resolution&& resolution,
				std::optional<Property::HDR10>&& hdr10) noexcept;

			/**
			 * Copy constructor.
			 */
			Video(const Video& other) = default;

			/**
			 * Move constructor.
			 */
			Video(Video&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Video() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Video& operator=(const Video& other) = default;

			/**
			 * Move assignment.
			 */
			Video& operator=(Video&& other) = default;

			/**
			 * @return Color properties.
			 */
			const Property::Color& Color() const noexcept;

			/**
			 * @return Resolution.
			 */
			const Property::Resolution& Resolution() const noexcept;

			/**
			 * @return Optional HDR10 data.
			 */
			const std::optional<Property::HDR10>& HDR10() const noexcept;

			/**
			 * @return Cloned context.
			 */
			PointerType Clone() const override;

			/**
			 * @return Moved context as new pointer.
			 */
			PointerType Move() override;

		private:
			Property::Color m_color;						///< Color properties
			Property::Resolution m_resolution;				///< Frame size
			std::optional<Property::HDR10> m_hdr10;			///< Optional HDR10
	};
}
