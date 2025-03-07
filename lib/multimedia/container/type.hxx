#pragma once

#include <multimedia/visibility.h>

#include <string>

/**
 * @namespace Container
 * @brief The namespace for all multimedia containers.
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @enum Type
	 * @brief The enum for container types.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Type: unsigned short {
		Matroska,		///< Matroska container.
		AVI,			///< AVI container.
		WebM,			///< WebM container.
		MP3,			///< MP3 container.
		WAV,			///< WAV container.
		OGG,			///< OGG container.
		OPUS,			///< OPUS container.
		FLAC,			///< FLAC container.
		Unknown			///< Unknown container.
	};

	/**
	 * @brief Converts a container type to a string.
	 * @param type The container type.
	 * @return The string representation of the container type.
	 */
	constexpr std::string STORMBYTE_MULTIMEDIA_PUBLIC TypeToString(const Type& type) noexcept {
		switch (type) {
			case Type::Matroska:	return "Matroska";
			case Type::AVI:			return "AVI";
			case Type::WebM:		return "WebM";
			case Type::MP3:			return "MP3";
			case Type::WAV:			return "WAV";
			case Type::OGG:			return "OGG";
			case Type::OPUS:		return "OPUS";
			case Type::FLAC:		return "FLAC";
			case Type::Unknown:		return "Unknown";
		}
	};
}