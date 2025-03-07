#pragma once

#include <multimedia/visibility.h>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia related classes.
 */
namespace StormByte::Multimedia {
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Type:unsigned short {
		Audio,
		Video,
		Image,
		Subtitle,
	};
}