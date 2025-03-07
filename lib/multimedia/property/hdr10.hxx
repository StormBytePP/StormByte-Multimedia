#pragma once

#include <multimedia/property/color.hxx>

#include <optional>
#include <string>

/**
 * @namespace Property
 * @brief The namespace for all multimedia properties.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @class HDR10
	 * @brief The class for HDR10 properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC HDR10 {
		public:
			static const HDR10 							Default;	///< The default HDR10 properties.

			/**
			 * @brief Default constructor.
			 * @param red The red point.
			 * @param green The green point.
			 * @param blue The blue point.
			 * @param white The white point.
			 * @param luminance The luminance.
			 * @param light_level The light level.
			 */
			HDR10(const Color::Point& red,
				const Color::Point& green,
				const Color::Point& blue,
				const Color::Point& white,
				const Color::Point& luminance,
				const std::optional<Color::Point>& light_level = std::nullopt
			);

			/**
			 * @brief Default constructor.
			 * @param red The red point.
			 * @param green The green point.
			 * @param blue The blue point.
			 * @param white The white point.
			 * @param luminance The luminance.
			 * @param light_level The light level.
			 */
			HDR10(Color::Point&& red,
				Color::Point&& green,
				Color::Point&& blue,
				Color::Point&& white,
				Color::Point&& luminance,
				std::optional<Color::Point>&& light_level = std::nullopt
			);

			/**
			 * @brief Copy constructor.
			 * @param hdr10 The HDR10 to copy.
			 */
			HDR10(const HDR10& hdr10)					= default;

			/**
			 * @brief Move constructor.
			 * @param hdr10 The HDR10 to move.
			 */
			HDR10(HDR10&& hdr10) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param hdr10 The HDR10 to copy.
			 * @return The copied HDR10.
			 */
			HDR10& operator=(const HDR10& hdr10)		= default;

			/**
			 * @brief Move assignment operator.
			 * @param hdr10 The HDR10 to move.
			 * @return The moved HDR10.
			 */
			HDR10& operator=(HDR10&& hdr10) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~HDR10() noexcept					= default;

			/**
			 * @brief Gets the red point.
			 * @return The red point.
			 */
			const Color::Point& 						GetRedPoint() const noexcept;

			/**
			 * @brief Sets the red point.
			 * @param red_point The red point.
			 */
			void 										SetRedPoint(const Color::Point&);

			/**
			 * @brief Sets the red point.
			 * @param red_point The red point.
			 */
			void 										SetRedPoint(Color::Point&&) noexcept;

			/**
			 * @brief Gets the green point.
			 * @return The green point.
			 */
			const Color::Point& 						GetGreenPoint() const noexcept;

			/**
			 * @brief Sets the green point.
			 * @param green_point The green point.
			 */
			void 										SetGreenPoint(const Color::Point&);

			/**
			 * @brief Sets the green point.
			 * @param green_point The green point.
			 */
			void 										SetGreenPoint(Color::Point&&) noexcept;

			/**
			 * @brief Gets the blue point.
			 * @return The blue point.
			 */
			const Color::Point& 						GetBluePoint() const noexcept;

			/**
			 * @brief Sets the blue point.
			 * @param blue_point The blue point.
			 */
			void 										SetBluePoint(const Color::Point&);

			/**
			 * @brief Sets the blue point.
			 * @param blue_point The blue point.
			 */
			void 										SetBluePoint(Color::Point&&) noexcept;

			/**
			 * @brief Gets the white point.
			 * @return The white point.
			 */
			const Color::Point& 						GetWhitePoint() const noexcept;

			/**
			 * @brief Sets the white point.
			 * @param white_point The white point.
			 */
			void 										SetWhitePoint(const Color::Point&);

			/**
			 * @brief Sets the white point.
			 * @param white_point The white point.
			 */
			void 										SetWhitePoint(Color::Point&&) noexcept;

			/**
			 * @brief Gets the luminance.
			 * @return The luminance.
			 */
			const Color::Point& 						GetLuminance() const noexcept;

			/**
			 * @brief Sets the luminance.
			 * @param luminance The luminance.
			 */
			void 										SetLuminance(const Color::Point&);

			/**
			 * @brief Sets the luminance.
			 * @param luminance The luminance.
			 */
			void 										SetLuminance(Color::Point&&) noexcept;

			/**
			 * @brief Gets the light level.
			 * @return The light level.
			 */
			const std::optional<Color::Point>& 			GetLightLevel() const noexcept;

			/**
			 * @brief Sets the light level.
			 * @param light_level The light level.
			 */
			void 										SetLightLevel(const Color::Point&);

			/**
			 * @brief Sets the light level.
			 * @param light_level The light level.
			 */
			void 										SetLightLevel(Color::Point&&) noexcept;

			/**
			 * @brief Checks if it is HDR10+ compatible.
			 * @return True if it is HDR10+ compatible, false otherwise.
			 */
			bool 										IsHDR10Plus() const noexcept;

			/**
			 * @brief Sets the HDR10+ status.
			 */
			void 										SetHDR10Plus() noexcept;

		private:
			Color::Point m_red;							///< The red point.
			Color::Point m_green;						///< The green point.
			Color::Point m_blue;						///< The blue point.
			Color::Point m_white;						///< The white point.
			Color::Point m_luminance;					///< The luminance.
			std::optional<Color::Point> m_light_level;	///< The light level.
			bool m_hdr10plus;							///< The HDR10+ status.
	};
}