#pragma once

#include <StormByte/multimedia/visibility.h>

#include <string>

/**
 * @namespace Media
 * @brief The namespace for all multimedia media types.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @struct Language
	 * @brief The struct for language properties.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Language {
		std::string	s_iso639;		///< The name of the language.
	};
}