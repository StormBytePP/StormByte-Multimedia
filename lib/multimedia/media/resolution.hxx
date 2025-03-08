#pragma once

#include <multimedia/visibility.h>

#include <string>

/**
 * @namespace Media
 * @brief The namespace for all multimedia media types.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @class Resolution
	 * @brief The struct for size properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Resolution final {
		public:
			/**
			 * @brief Constructor.
			 * @param width The width of the resolution.
			 * @param height The height of the resolution.
			 */
			constexpr Resolution(unsigned short width, unsigned short height):
				m_width(width), m_height(height) {}
			
			/**
			 * @brief Copy constructor.
			 * @param resolution The Resolution to copy.
			 */
			constexpr Resolution(const Resolution& resolution) 					= default;

			/**
			 * @brief Move constructor.
			 * @param resolution The Resolution to move.
			 */
			constexpr Resolution(Resolution&& resolution) noexcept 				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param resolution The Resolution to copy.
			 * @return The copied Resolution.
			 */
			constexpr Resolution& operator=(const Resolution& resolution) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param resolution The Resolution to move.
			 * @return The moved Resolution.
			 */
			constexpr Resolution& operator=(Resolution&& resolution) noexcept	= default;

			/**
			 * @brief Default destructor.
			 */
			constexpr ~Resolution() noexcept 									= default;

			/**
			 * @brief Gets the width of the resolution.
			 * @return The width of the resolution.
			 */
			constexpr unsigned short 											GetWidth() const noexcept {
				return m_width;
			}

			/**
			 * @brief Gets the height of the resolution.
			 * @return The height of the resolution.
			 */
			constexpr unsigned short 											GetHeight() const noexcept {
				return m_height;
			}

			/**
			 * @brief Gets the name of the resolution.
			 * @return The name of the resolution. (Example 1920x1080)
			 */
			std::string 														GetName() const noexcept;

			/**
			 * @brief Gets the standard name of the resolution.
			 * @return The standard name of the resolution. (Example 1080p)
			 */
			std::string 														GetStandardName() const noexcept;

		private:
			unsigned short m_width;												///< The width of the resolution.
			unsigned short m_height;											///< The height of the resolution.
	};
}