#pragma once

#include <StormByte/multimedia/visibility.h>

#include <string>
#include <utility>

/**
 * @namespace Video
 * @brief The namespace for multimedia video media properties.
 */
namespace StormByte::Multimedia::Media::Property::Video {
	/**
	 * @class Color
	 * @brief The class for color properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Color {
		public:
			using Point = std::pair<int, int>;			///< Representation for a point

			/**
			 * @brief Constructor.
			 * @param pix_fmt The pixel format.
			 * @param range The range.
			 * @param space The space.
			 */
			Color(const std::string& pix_fmt, const std::string& range, const std::string& space) noexcept;

			/**
			 * @brief Constructor.
			 * @param pix_fmt The pixel format.
			 * @param range The range.
			 * @param space The space.
			 */
			Color(std::string&& pix_fmt, std::string&& range, std::string&& space) noexcept;

			/**
			 * @brief Copy constructor
			 * @param color The Color to copy.
			 */
			Color(const Color& color)					= default;

			/**
			 * @brief Move constructor.
			 * @param color The Color to move.
			 */
			Color(Color&& color) noexcept				= default;

			/**
			 * Copy assignment operator.
			 * @param color The Color to copy.
			 * @return The copied Color.
			 */
			Color& operator=(const Color& color)		= default;

			/**
			 * Move assignment operator.
			 * @param color The Color to move.
			 * @return The moved Color.
			 */
			Color& operator=(Color&& color) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Color() noexcept					= default;

			/**
			 * @brief Gets the primaries.
			 * @return The primaries.
			 */
			const std::string&							PixelFormat() const noexcept;

			/**
			 * @brief Sets the color range.
			 */
			const std::string&							Range() const noexcept;

			/**
			 * @brief Gets the matrix.
			 * @return The matrix.
			 */
			const std::string&							Space() const noexcept;

			/**
			 * @brief Checks if HDR10 is possible.
			 * @return True if HDR10 is possible, false otherwise.
			 */
			constexpr virtual bool 						IsHDR10() const noexcept {
				return false;
			}

		protected:
			std::string m_pix_fmt;						///< The pixel format.
			std::string m_range;						///< The range.
			std::string m_space;						///< The space.
	};
}