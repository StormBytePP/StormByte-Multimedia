#pragma once

#include <StormByte/alias.hxx>
#include <StormByte/multimedia/exception.hxx>
#include <StormByte/multimedia/media/property/video/color.hxx>

#include <optional>

/**
 * @namespace Video
 * @brief The namespace for multimedia video media properties.
 */
namespace StormByte::Multimedia::Media::Property::Video {
	/**
	 * @class HDR10
	 * @brief The class for HDR10 properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC HDR10 final: public Color {
		public:
			static const HDR10 									Default;	///< The default HDR10 properties.

			/**
			 * @brief Constructor with default HDR color points
			 * @param pix_fmt The pixel format.
			 * @param range The range.
			 * @param space The space.
			 * @param primaries The primaries.
			 * @param transfer The transfer.
			 */
			HDR10(const std::string& pix_fmt, const std::string& range, const std::string& space, const std::string& primaries, const std::string& transfer) noexcept;

			/**
			 * @brief Default constructor.
			 * @param pix_fmt The pixel format.
			 * @param range The range.
			 * @param space The space.
			 * @param primaries The primaries.
			 * @param transfer The transfer.
			 * @param red The red point.
			 * @param green The green point.
			 * @param blue The blue point.
			 * @param white The white point.
			 * @param luminance The luminance.
			 * @param light_level The light level.
			 */
			HDR10(const std::string& pix_fmt, const std::string& range, const std::string& space, const std::string& primaries, const std::string& transfer,
				const Color::Point& red,
				const Color::Point& green,
				const Color::Point& blue,
				const Color::Point& white,
				const Color::Point& luminance,
				const std::optional<Color::Point>& light_level = std::nullopt
			) noexcept;

			/**
			 * @brief Default constructor.
			 * @param pix_fmt The pixel format.
			 * @param range The range.
			 * @param space The space.
			 * @param primaries The primaries.
			 * @param transfer The transfer.
			 * @param red The red point.
			 * @param green The green point.
			 * @param blue The blue point.
			 * @param white The white point.
			 * @param luminance The luminance.
			 * @param light_level The light level.
			 */
			HDR10(std::string&& pix_fmt, std::string&& range, std::string&& space, std::string&& primaries, std::string&& transfer,
				Color::Point&& red,
				Color::Point&& green,
				Color::Point&& blue,
				Color::Point&& white,
				Color::Point&& luminance,
				std::optional<Color::Point>&& light_level = std::nullopt
			) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param hdr10 The HDR10 to copy.
			 */
			HDR10(const HDR10& hdr10)							= default;

			/**
			 * @brief Move constructor.
			 * @param hdr10 The HDR10 to move.
			 */
			HDR10(HDR10&& hdr10) noexcept						= default;

			/**
			 * @brief Copy assignment operator.
			 * @param hdr10 The HDR10 to copy.
			 * @return The copied HDR10.
			 */
			HDR10& operator=(const HDR10& hdr10)				= default;

			/**
			 * @brief Move assignment operator.
			 * @param hdr10 The HDR10 to move.
			 * @return The moved HDR10.
			 */
			HDR10& operator=(HDR10&& hdr10) noexcept			= default;

			/**
			 * @brief Default destructor.
			 */
			~HDR10() noexcept override							= default;

			/**
			 * @brief Gets color primaries
			 * @return The color primaries
			 */
			const std::string& 									Primaries() const noexcept;

			/**
			 * @brief Gets transfer function
			 * @return The transfer function
			 */
			const std::string& 									Transfer() const noexcept;

			/**
			 * @brief Gets the red point.
			 * @return The red point.
			 */
			const Color::Point& 								RedPoint() const noexcept;

			/**
			 * @brief Gets the green point.
			 * @return The green point.
			 */
			const Color::Point& 								GreenPoint() const noexcept;

			/**
			 * @brief Gets the blue point.
			 * @return The blue point.
			 */
			const Color::Point& 								BluePoint() const noexcept;

			/**
			 * @brief Gets the white point.
			 * @return The white point.
			 */
			const Color::Point& 								WhitePoint() const noexcept;

			/**
			 * @brief Gets the luminance.
			 * @return The luminance.
			 */
			const Color::Point& 								Luminance() const noexcept;

			/**
			 * @brief Gets the light level.
			 * @return The light level.
			 */
			const std::optional<Color::Point>& 					LightLevel() const noexcept;

			/**
			 * @brief Checks if HDR10 is possible.
			 * @return True if HDR10 is possible, false otherwise.
			 */
			constexpr bool 										IsHDR10() const noexcept override {
				return m_primaries == Default.m_primaries && m_space == Default.m_space
						&& m_transfer == Default.m_transfer && m_pix_fmt == Default.m_pix_fmt;
			}

			/**
			 * @brief Checks if it is HDR10+ compatible.
			 * @return True if it is HDR10+ compatible, false otherwise.
			 */
			bool 												IsHDR10Plus() const noexcept;

			/**
			 * @brief Sets HDR10+ compatible status.
			 * @param hdrplus The HDR10+ compatible status.
			 */
			void 												HDR10Plus(bool hdrplus) noexcept;

			/**
			 * @brief Gets color point from string like "6550/50000"
			 * @param str The string to parse
			 * @return The color point
			 */
			static StormByte::Expected<Color::Point, Multimedia::Exception>	ColorPoint(const std::string& fraction_x, const std::string& fraction_y) noexcept;

			/**
			 * @brief Gets color point from string like "6550/50000"
			 * @param str The string to parse
			 * @return The color point
			 */
			static StormByte::Expected<Color::Point, Multimedia::Exception>	LuminanceColorPoint(const std::string& fraction_x, const std::string& fraction_y) noexcept;

		private:
			std::string m_primaries;							///< The primaries.
			std::string m_transfer;								///< The transfer.
			Color::Point m_red;									///< The red point.
			Color::Point m_green;								///< The green point.
			Color::Point m_blue;								///< The blue point.
			Color::Point m_white;								///< The white point.
			Color::Point m_luminance;							///< The luminance.
			std::optional<Color::Point> m_light_level;			///< The light level.
			bool m_hdr10plus;									///< The HDR10+ status.
	};
}