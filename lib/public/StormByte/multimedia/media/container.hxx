#pragma once

#include <StormByte/alias.hxx>
#include <StormByte/multimedia/exception.hxx>
#include <StormByte/multimedia/media/codec.hxx>
#include <StormByte/multimedia/media/type.hxx>

#include <optional>
#include <string>
#include <vector>
#include <unordered_map>

/**
 * @namespace Container
 * @brief The namespace for all multimedia media container information.
 */
namespace StormByte::Multimedia::Media::Container {
	/**
	 * @enum Container
	 * @brief Enumerates all supported multimedia containers.
	 */
	enum STORMBYTE_MULTIMEDIA_PUBLIC Name: unsigned short {
		// Audio Containers
		AC3,		///< **AC3**: Audio Codec 3 container, commonly used for surround sound audio files.
		FLAC,		///< **FLAC**: Free Lossless Audio Codec, often used for high-quality audio storage.
		MP3,		///< **MP3**: MPEG-1 Audio Layer III, a widely used lossy audio format.
		OGA,		///< **OGA**: Ogg Audio container, typically used with Vorbis or Opus codecs.
		WAV,		///< **WAV**: Waveform Audio File Format, used for uncompressed or PCM audio.

		// Video Containers
		AVI,		///< **AVI**: Audio Video Interleave container, one of the oldest multimedia container formats.
		MKV,		///< **MKV**: Matroska container, a flexible format that supports subtitles, audio, and video.
		MP4,		///< **MP4**: MPEG-4 container, one of the most widely used formats for video playback.
		M2TS,		///< **M2TS**: MPEG-2 Transport Stream, used for Blu-ray and AVCHD video storage.
		MPEG,		///< **MPEG**: MPEG container, used for both video and audio in digital TV and DVDs.
		MPG,		///< **MPG**: MPEG-1 container, an earlier standard for video storage and playback.
		OGV,		///< **OGV**: Ogg Video container, commonly used with Theora and Vorbis codecs.
		TS,			///< **TS**: MPEG Transport Stream container, designed for broadcasting systems.
		WEBM,		///< **WEBM**: WebM container, optimized for web video playback with VP8/VP9 codecs.

		// Image Containers
		BMP,		///< **BMP**: Bitmap container, a raster graphics format for storing digital images.
		GIF,		///< **GIF**: Graphics Interchange Format, widely used for animations and simple images.
		HEIC,		///< **HEIC**: High Efficiency Image Container, used for storing HEIF images.
		JPG,		///< **JPG**: JPEG container, the most popular format for lossy image compression.
		PNG			///< **PNG**: Portable Network Graphics, a lossless image format supporting transparency.
	};

	/**
	 * @struct Info
	 * @brief Holds information about a container.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Info {
		std::string s_name;								///< The name of the container (e.g., "MP4").
		std::optional<unsigned short> s_max_streams;	///< The maximum number of streams allowed in the container.
		std::vector<Codec::Name> s_allowed_codecs;		///< The list of codecs allowed in the container.
	};

	/**
	 * @class Registry
	 * @brief Centralized registry for managing container metadata.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Registry {
		/**
		 * @brief Retrieves information about a container.
		 * @param container The container enum value.
		 * @return Reference to the Info struct of the container.
		 */
		static const Info& Info(const Name& container);

		/**
		 * @brief Retrieves the container enum by its name.
		 * @param name The container name (case-insensitive).
		 * @return The corresponding container enum value.
		 * @throws std::out_of_range If the container name is not found.
		 */
		static StormByte::Expected<Name, ContainerNotFound> Info(const std::string& name);

	private:
		static const std::unordered_map<Name, Container::Info> c_container_registry;	///< Registry of container metadata.
		static const std::unordered_map<std::string, Name> c_container_name_map;		///< Reverse lookup map.
	};
}