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
