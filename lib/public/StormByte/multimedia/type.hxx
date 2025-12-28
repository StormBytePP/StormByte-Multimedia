#pragma once

#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @enum Type
	 * @brief The enumeration representing different types of media.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Type {
		Audio,							///< Audio media type
		Video,							///< Video media type
		Subtitle,						///< Text media type
		Attachment,						///< Attachment media type
		Copy,							///< Copy media type
		Unknown							///< Unknown media type
	};

	/**
	 * @brief Converts a Type enum to its string representation.
	 * @param type The Type enum value.
	 * @return The string representation of the Type enum.
	 */
	constexpr const char* ToString(Type type) noexcept {
		switch (type) {
			case Type::Audio:			return "Audio";
			case Type::Video:			return "Video";
			case Type::Subtitle:		return "Subtitle";
			case Type::Attachment:		return "Attachment";
			case Type::Copy:			return "Copy";
			case Type::Unknown:			return "Unknown";
			default:					return "Invalid";
		}
	}
}