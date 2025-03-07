#pragma once

#include <multimedia/visibility.h>

/**
 * @namespace Property
 * @brief The namespace for all multimedia properties.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @enum Type
	 * @brief The enum for all multimedia types.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Type:unsigned short {
		Audio,		///< The audio type.
		Video,		///< The video type.
		Image,		///< The image type.
		Subtitle,	///< The subtitle type.
	};
}