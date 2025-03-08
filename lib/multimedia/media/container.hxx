#pragma once

#include <multimedia/visibility.h>

#include <string>

/**
 * @namespace Media
 * @brief The namespace for all multimedia media classes.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @enum Container
	 * @brief The container name.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Container: unsigned short {
		// Audio
		AC3, 			///< Audio Codec 3 container.
		FLAC,			///< Free Lossless Audio Codec container.
		MP3, 			///< MPEG-1 Audio Layer III container.
		OGA, 			///< Ogg Audio container.
		WAV, 			///< Waveform Audio File Format container.
	
		// Video
		AVI, 			///< Audio Video Interleave container.
		MKV, 			///< Matroska container.
		MP4, 			///< MPEG-4 container.
		M2TS,			///< MPEG-2 Transport Stream container.
		MPEG,			///< MPEG container.
		MPG, 			///< MPEG-1 container.
		OGV, 			///< Ogg Video container.
		TS,  			///< MPEG Transport Stream container.
		WEBM,			///< WebM container.
	
		// Image
		BMP, 			///< Bitmap container.
		GIF, 			///< Graphics Interchange Format container.
		HEIC,			///< High Efficiency Image Container.
		JPG, 			///< JPEG container.
		PNG, 			///< Portable Network Graphics container.
	
		// Unknown
		Unknown			///< Unknown container.
	};

	/**
	 * @brief Gets the type of the container.
	 * @param extension The file extension.
	 * @return The type of the container.
	 */
	Container STORMBYTE_MULTIMEDIA_PUBLIC ContainerFromExtension(const std::string& extension) noexcept;
}