/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Property
 * @brief Video/audio property value types.
 */
namespace StormByte::Multimedia::Context::Property {
	/**
	 * @class Point
	 * @brief Integer 2D point (e.g. chromaticity / luminance pair).
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Point final {
		public:
			/**
			 * @param x X coordinate.
			 * @param y Y coordinate.
			 */
			Point(int x, int y) noexcept;

			/**
			 * Copy constructor.
			 */
			Point(const Point& point) = default;

			/**
			 * Move constructor.
			 */
			Point(Point&& point) noexcept = default;

			/**
			 * Copy assignment.
			 */
			Point& operator=(const Point& point) = default;

			/**
			 * Move assignment.
			 */
			Point& operator=(Point&& point) noexcept = default;

			/**
			 * Destructor.
			 */
			~Point() noexcept = default;

			/**
			 * @return X coordinate.
			 */
			int X() const noexcept;

			/**
			 * @return Y coordinate.
			 */
			int Y() const noexcept;

			/**
			 * Builds a point scaled to a common denominator.
			 * @param numerator_x X numerator.
			 * @param denominator_x X denominator.
			 * @param numerator_y Y numerator.
			 * @param denominator_y Y denominator.
			 * @param denominator Target scale denominator.
			 * @return Normalized point.
			 */
			static Point Normalized(int numerator_x, int denominator_x, int numerator_y, int denominator_y, int denominator) noexcept;

		protected:
			int m_x;	///< X
			int m_y;	///< Y
	};
}
