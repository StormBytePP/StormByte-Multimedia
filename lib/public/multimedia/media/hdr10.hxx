#pragma once

#include <multimedia/media/color.hxx>

#include <optional>
#include <string>

/**
 * @namespace Media
 * @brief The namespace for all multimedia media types.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @class HDR10
	 * @brief The class for HDR10 properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC HDR10 {
		public:
			static const HDR10 									Default;	///< The default HDR10 properties.

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
			constexpr HDR10(const HDR10& hdr10)					= default;

			/**
			 * @brief Move constructor.
			 * @param hdr10 The HDR10 to move.
			 */
			constexpr HDR10(HDR10&& hdr10) noexcept				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param hdr10 The HDR10 to copy.
			 * @return The copied HDR10.
			 */
			constexpr HDR10& operator=(const HDR10& hdr10)		= default;

			/**
			 * @brief Move assignment operator.
			 * @param hdr10 The HDR10 to move.
			 * @return The moved HDR10.
			 */
			constexpr HDR10& operator=(HDR10&& hdr10) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			constexpr ~HDR10() noexcept							= default;

			/**
			 * @brief Gets the red point.
			 * @return The red point.
			 */
			inline const Color::Point& 							GetRedPoint() const noexcept {
				return m_red;
			}

			/**
			 * @brief Sets the red point.
			 * @param red_point The red point.
			 */
			inline void 										SetRedPoint(const Color::Point& red_point) {
				m_red = red_point;
			}

			/**
			 * @brief Sets the red point.
			 * @param red_point The red point.
			 */
			inline void 										SetRedPoint(Color::Point&& red_point) noexcept {
				m_red = std::move(red_point);
			}

			/**
			 * @brief Gets the green point.
			 * @return The green point.
			 */
			inline const Color::Point& 							GetGreenPoint() const noexcept {
				return m_green;
			}

			/**
			 * @brief Sets the green point.
			 * @param green_point The green point.
			 */
			inline void 										SetGreenPoint(const Color::Point& green_point) {
				m_green = green_point;
			}

			/**
			 * @brief Sets the green point.
			 * @param green_point The green point.
			 */
			inline void 										SetGreenPoint(Color::Point&& green_point) noexcept {
				m_green = std::move(green_point);
			}

			/**
			 * @brief Gets the blue point.
			 * @return The blue point.
			 */
			inline const Color::Point& 							GetBluePoint() const noexcept {
				return m_blue;
			}

			/**
			 * @brief Sets the blue point.
			 * @param blue_point The blue point.
			 */
			inline void 										SetBluePoint(const Color::Point& blue_point) {
				m_blue = blue_point;
			}

			/**
			 * @brief Sets the blue point.
			 * @param blue_point The blue point.
			 */
			inline void 										SetBluePoint(Color::Point&& blue_point) noexcept {
				m_blue = std::move(blue_point);
			}

			/**
			 * @brief Gets the white point.
			 * @return The white point.
			 */
			inline const Color::Point& 							GetWhitePoint() const noexcept {
				return m_white;
			}

			/**
			 * @brief Sets the white point.
			 * @param white_point The white point.
			 */
			inline void 										SetWhitePoint(const Color::Point& white_point) {
				m_white = white_point;
			}

			/**
			 * @brief Sets the white point.
			 * @param white_point The white point.
			 */
			inline void 										SetWhitePoint(Color::Point&& white_point) noexcept {
				m_white = std::move(white_point);
			}

			/**
			 * @brief Gets the luminance.
			 * @return The luminance.
			 */
			inline const Color::Point& 							GetLuminance() const noexcept {
				return m_luminance;
			}

			/**
			 * @brief Sets the luminance.
			 * @param luminance The luminance.
			 */
			inline void 										SetLuminance(const Color::Point& luminance) {
				m_luminance = luminance;
			}

			/**
			 * @brief Sets the luminance.
			 * @param luminance The luminance.
			 */
			inline void 										SetLuminance(Color::Point&& luminance) noexcept {
				m_luminance = std::move(luminance);
			}

			/**
			 * @brief Gets the light level.
			 * @return The light level.
			 */
			inline const std::optional<Color::Point>& 			GetLightLevel() const noexcept {
				return m_light_level;
			}

			/**
			 * @brief Sets the light level.
			 * @param light_level The light level.
			 */
			inline void 										SetLightLevel(const Color::Point& light_level) {
				m_light_level = light_level;
			}

			/**
			 * @brief Sets the light level.
			 * @param light_level The light level.
			 */
			inline void 										SetLightLevel(Color::Point&& light_level) noexcept {
				m_light_level = std::move(light_level);
			}

			/**
			 * @brief Checks if it is HDR10+ compatible.
			 * @return True if it is HDR10+ compatible, false otherwise.
			 */
			inline bool 										IsHDR10Plus() const noexcept {
				return m_hdr10plus;
			}

			/**
			 * @brief Sets the HDR10+ status.
			 */
			inline void 										SetHDR10Plus() noexcept	{
				m_hdr10plus = true;
			}

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