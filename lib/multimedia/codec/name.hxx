#pragma once

#include <multimedia/visibility.h>

#include <string>

/**
 * @namespace Codec
 * @brief The namespace for all codecs.
 */
namespace StormByte::Multimedia::Codec {
	/**
	 * @enum Name
	 * @brief The enumeration for all codec names.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Name: unsigned short {
		HEVC,
		H264,
		VP9,
		VP8,
		AV1,
		AVC,
		MP3,
		AAC,
		AC3,
		FLAC,
		OPUS,
		VORBIS,
		PCM,
		MJPEG,   // Motion JPEG
		THEORA,  // Theora codec
		DTS,     // DTS audio codec
		EAC3     // Enhanced AC-3
	};
	

	/**
	 * @brief Converts a codec name to a string.
	 * @param name The codec name.
	 * @return The string representation of the codec name.
	 */
	constexpr std::string STORMBYTE_MULTIMEDIA_PUBLIC NameToString(Name name) {
		switch (name) {
			case Name::HEVC: 	return "HEVC";
			case Name::H264: 	return "H264";
			case Name::VP9: 	return "VP9";
			case Name::VP8: 	return "VP8";
			case Name::AV1: 	return "AV1";
			case Name::AVC: 	return "AVC";
			default: 			return "Unknown";
		}
	}
}