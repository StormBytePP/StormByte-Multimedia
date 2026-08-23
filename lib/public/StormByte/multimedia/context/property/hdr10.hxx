/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia is dual-licensed under the following terms:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You can redistribute it and/or modify it under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this software may be used under the terms of a
 *    commercial license agreement with the sole copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *    Contact the copyright holder for more information.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this StormByte-Multimedia. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/multimedia/context/property/point.hxx>

#include <optional>

/**
 * @namespace Property
 * @brief Video/audio property value types.
 */
namespace StormByte::Multimedia::Context::Property {
	/**
	 * @class HDR10
	 * @brief Mastering display and content light level metadata.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC HDR10 final {
		public:
			/**
			 * Uses DEFAULT primaries / luminance.
			 */
			HDR10() noexcept;

			/**
			 * @param red Red primary.
			 * @param green Green primary.
			 * @param blue Blue primary.
			 * @param white White point.
			 * @param luminance Min/max luminance pair.
			 * @param light_level Optional MaxCLL/MaxFALL.
			 */
			HDR10(const Point& red, const Point& green, const Point& blue, const Point& white,
				const Point& luminance, const std::optional<Point>& light_level = std::nullopt) noexcept;

			/**
			 * Move overload of the full constructor.
			 */
			HDR10(Point&& red, Point&& green, Point&& blue, Point&& white,
				Point&& luminance, std::optional<Point>&& light_level = std::nullopt) noexcept;

			/**
			 * Copy constructor.
			 */
			HDR10(const HDR10& hdr10) = default;

			/**
			 * Move constructor.
			 */
			HDR10(HDR10&& hdr10) noexcept = default;

			/**
			 * Copy assignment.
			 */
			HDR10& operator=(const HDR10& hdr10) = default;

			/**
			 * Move assignment.
			 */
			HDR10& operator=(HDR10&& hdr10) noexcept = default;

			/**
			 * Destructor.
			 */
			~HDR10() noexcept = default;

			/**
			 * @return Red primary.
			 */
			const Point& Red() const noexcept;

			/**
			 * @return Green primary.
			 */
			const Point& Green() const noexcept;

			/**
			 * @return Blue primary.
			 */
			const Point& Blue() const noexcept;

			/**
			 * @return White point.
			 */
			const Point& White() const noexcept;

			/**
			 * @return Luminance (min, max).
			 */
			const Point& Luminance() const noexcept;

			/**
			 * @return Optional content light level (MaxCLL, MaxFALL).
			 */
			const std::optional<Point>& LightLevel() const noexcept;

			/**
			 * @return true if HDR10+ dynamic metadata was detected.
			 */
			bool IsHDR10Plus() const noexcept;

			/**
			 * Sets HDR10+ flag.
			 * @param hdrplus New value.
			 */
			void HDR10Plus(bool hdrplus) noexcept;

			/**
			 * Default mastering display constants when source metadata is missing.
			 */
			static const HDR10 DEFAULT;

		private:
			Point m_red;							///< Red primary
			Point m_green;							///< Green primary
			Point m_blue;							///< Blue primary
			Point m_white;							///< White point
			Point m_luminance;						///< Min/max luminance
			std::optional<Point> m_light_level;		///< MaxCLL / MaxFALL
			bool m_hdr10plus;						///< HDR10+ present
	};
}
