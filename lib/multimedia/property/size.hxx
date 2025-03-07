#pragma once

#include <multimedia/visibility.h>

#include <cstdint>
#include <string>

/**
 * @namespace Property
 * @brief The namespace for all multimedia properties.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @struct Size
	 * @brief The struct for size properties.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Size {
		uint64_t s_bytes;		///< The size in bytes.
		/**
		 * @brief Gets size as string (example: 10.5GiB)
		 */
		std::string 			ToString() const noexcept;
	};
}