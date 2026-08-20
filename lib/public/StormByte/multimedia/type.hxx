#pragma once

#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @enum Type
	 * @brief Media stream / content type.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Type {
		Audio,		///< Audio media type
		Video,		///< Video media type
		Subtitle,	///< Subtitle media type
		Attachment,	///< Attachment media type
		Copy,		///< Stream copy (passthrough)
		Unknown		///< Unknown media type
	};

	/**
	 * Converts a Type to a string.
	 * @param type Value to convert.
	 * @return Null-terminated string literal.
	 */
	constexpr const char* ToString(Type type) noexcept {
		switch (type) {
			case Type::Audio:		return "Audio";
			case Type::Video:		return "Video";
			case Type::Subtitle:	return "Subtitle";
			case Type::Attachment:	return "Attachment";
			case Type::Copy:		return "Copy";
			case Type::Unknown:		return "Unknown";
			default:				return "Invalid";
		}
	}
}
