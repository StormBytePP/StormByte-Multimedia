#pragma once

#include <StormByte/multimedia/visibility.h>

#include <string>

/**
 * @namespace Property
 * @brief The namespace for all context properties.
 */
namespace StormByte::Multimedia::Context::Property {
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
			Resolution(unsigned short width, unsigned short height);
			
			/**
			 * @brief Copy constructor.
			 * @param resolution The Resolution to copy.
			 */
			Resolution(const Resolution& resolution) 						= default;

			/**
			 * @brief Move constructor.
			 * @param resolution The Resolution to move.
			 */
			Resolution(Resolution&& resolution) noexcept 					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param resolution The Resolution to copy.
			 * @return The copied Resolution.
			 */
			Resolution& operator=(const Resolution& resolution) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param resolution The Resolution to move.
			 * @return The moved Resolution.
			 */
			Resolution& operator=(Resolution&& resolution) noexcept			= default;

			/**
			 * @brief Default destructor.
			 */
			~Resolution() noexcept 											= default;

			/**
			 * @brief Gets the width of the resolution.
			 * @return The width of the resolution.
			 */
			unsigned short 													Width() const noexcept;

			/**
			 * @brief Gets the height of the resolution.
			 * @return The height of the resolution.
			 */
			unsigned short 													Height() const noexcept;

			/**
			 * @brief Gets the name of the resolution.
			 * @return The name of the resolution. (Example 1920x1080)
			 */
			std::string 													Name() const noexcept;

			/**
			 * @brief Gets the standard name of the resolution.
			 * @return The standard name of the resolution. (Example 1080p)
			 */
			std::string 													StandardName() const noexcept;

		private:
			unsigned short m_width;											///< The width of the resolution.
			unsigned short m_height;										///< The height of the resolution.
	};
}