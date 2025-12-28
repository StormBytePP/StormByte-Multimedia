#pragma once

#include <StormByte/multimedia/context/property/point.hxx>

#include <optional>

/**
 * @namespace Property
 * @brief The namespace for all context properties.
 */
namespace StormByte::Multimedia::Context::Property {
	/**
	 * @class HDR10
	 * @brief The class for HDR10 properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC HDR10 final {
		public:
			/**
			 * @brief Default constructor
			 */
			HDR10() noexcept;

			/**
			 * @brief Constructor.
			 * @param red The red point.
			 * @param green The green point.
			 * @param blue The blue point.
			 * @param white The white point.
			 * @param luminance The luminance.
			 * @param light_level The light level.
			 */
			HDR10(const Point& red, const Point& green, const Point& blue, const Point& white, const Point& luminance, const std::optional<Point>& light_level = std::nullopt) noexcept;

			/**
			 * @brief Constructor.
			 * @param red The red point.
			 * @param green The green point.
			 * @param blue The blue point.
			 * @param white The white point.
			 * @param luminance The luminance.
			 * @param light_level The light level.
			 */
			HDR10(Point&& red, Point&& green, Point&& blue, Point&& white, Point&& luminance, std::optional<Point>&& light_level = std::nullopt) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param hdr10 The HDR10 to copy.
			 */
			HDR10(const HDR10& hdr10)										= default;

			/**
			 * @brief Move constructor.
			 * @param hdr10 The HDR10 to move.
			 */
			HDR10(HDR10&& hdr10) noexcept									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param hdr10 The HDR10 to copy.
			 * @return The copied HDR10.
			 */
			HDR10& operator=(const HDR10& hdr10)							= default;

			/**
			 * @brief Move assignment operator.
			 * @param hdr10 The HDR10 to move.
			 * @return The moved HDR10.
			 */
			HDR10& operator=(HDR10&& hdr10) noexcept						= default;

			/**
			 * @brief Default destructor.
			 */
			~HDR10() noexcept 												= default;

			/**
			 * @brief Gets the red point.
			 * @return The red point.
			 */
			const Point& 													Red() const noexcept;

			/**
			 * @brief Gets the green point.
			 * @return The green point.
			 */
			const Point& 													Green() const noexcept;

			/**
			 * @brief Gets the blue point.
			 * @return The blue point.
			 */
			const Point& 													Blue() const noexcept;

			/**
			 * @brief Gets the white point.
			 * @return The white point.
			 */
			const Point& 													White() const noexcept;

			/**
			 * @brief Gets the luminance.
			 * @return The luminance.
			 */
			const Point& 													Luminance() const noexcept;

			/**
			 * @brief Gets the light level.
			 * @return The light level.
			 */
			const std::optional<Point>& 									LightLevel() const noexcept;

			/**
			 * @brief Checks if it is HDR10+ compatible.
			 * @return True if it is HDR10+ compatible, false otherwise.
			 */
			bool 															IsHDR10Plus() const noexcept;

			/**
			 * @brief Sets HDR10+ compatible status.
			 * @param hdrplus The HDR10+ compatible status.
			 */
			void 															HDR10Plus(bool hdrplus) noexcept;

			/**
			 * @brief The default HDR10 properties.
			 */
			static const HDR10 DEFAULT;										///< The default HDR10 properties.

		private:
			Point m_red;													///< The red point.
			Point m_green;													///< The green point.
			Point m_blue;													///< The blue point.
			Point m_white;													///< The white point.
			Point m_luminance;												///< The luminance.
			std::optional<Point> m_light_level;								///< The light level.
			bool m_hdr10plus;												///< The HDR10+ status.
	};
}