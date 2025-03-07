#include <multimedia/visibility.h>

#include <string>

/**
 * @namespace Property
 * @brief The namespace for all multimedia properties.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @struct Language
	 * @brief The struct for language properties.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Language {
		std::string	s_iso639;		///< The name of the language.
	};
}