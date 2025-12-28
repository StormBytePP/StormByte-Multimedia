#pragma once

#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Property
 * @brief The namespace for all context properties.
 */
namespace StormByte::Multimedia::Context::Property {
	/**
	 * @class Point
	 * @brief Represents a color point in BT.2020 color space.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Point final {
		public:
			/**
			 * @brief Constructor.
			 * @param x The x coordinate.
			 * @param y The y coordinate.
			 */
			Point(int x, int y) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param point The Point to copy.
			 */
			Point(const Point& point)									= default;

			/**
			 * @brief Move constructor.
			 * @param point The Point to move.
			 */
			Point(Point&& point) noexcept								= default;

			/**
			 * @brief Copy assignment operator.
			 * @param point The Point to copy.
			 * @return The copied Point.
			 */
			Point& operator=(const Point& point)						= default;

			/**
			 * @brief Move assignment operator.
			 * @param point The Point to move.
			 * @return The moved Point.
			 */
			Point& operator=(Point&& point) noexcept					= default;

			/**
			 * @brief Default destructor.
			 */
			~Point() noexcept 											= default;

			/**
			 * @brief Gets the x coordinate.
			 * @return The x coordinate.
			 */
			int															X() const noexcept;

			/**
			 * @brief Gets the y coordinate.
			 * @return The y coordinate.
			 */
			int															Y() const noexcept;

			/**
			 * @brief Creates a normalized Point.
			 * @param numerator_x The numerator for the x coordinate.
			 * @param denominator_x The denominator for the x coordinate.
			 * @param numerator_y The numerator for the y coordinate.
			 * @param denominator_y The denominator for the y coordinate.
			 * @param denominator The common denominator.
			 * @return The normalized Point.
			 */
			static Point 												Normalized(int numerator_x, int denominator_x, int numerator_y, int denominator_y, int denominator) noexcept;

		protected:
			int m_x;													///< The x coordinate.
			int m_y;													///< The y coordinate.
	};
}