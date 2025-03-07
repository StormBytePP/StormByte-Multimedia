#pragma once

#include <multimedia/visibility.h>

#include <string>

/**
 * @namespace Property
 * @brief The namespace for all multimedia properties.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @struct Resolution
	 * @brief The struct for size properties.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Resolution {
		unsigned int	s_width;	///< The width of the size.
		unsigned int	s_height;	///< The height of the size.
		/**
		 * @brief Gets the name of the size.
		 * @return The name of the size. (Example 1920x1080)
		 */
		std::string		GetName() const noexcept;

		/**
		 * @brief Gets the standard name of the size.
		 * @return The standard name of the size. (Example 1080p)
		 */
		std::string		GetStandardName() const noexcept;
	};
}