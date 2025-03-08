#pragma once

#include <multimedia/visibility.h>

/**
 * @namespace Media
 * @brief The namespace for all multimedia media types.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @enum StreamType
	 * @brief The type of the stream.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Type: unsigned short {
		Audio,			///< Audio stream.
		Video,			///< Video stream.
		Subtitle,		///< Subtitle stream.
		Image,			///< Image stream.
		Attachment,		///< Attachment stream.
		Unknown			///< Unknown stream.
	};
}