#pragma once

#include <StormByte/alias.hxx>
#include <StormByte/multimedia/exception.hxx>
#include <StormByte/multimedia/media/codec.hxx>
#include <StormByte/multimedia/media/type.hxx>

#include <string>
#include <vector>

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
	 * @class Info
	 * @brief Holds information about a container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Info {
		public:
			using PointerType = std::shared_ptr<Info>;				///< Shared pointer to Info.
			using ConstPointerType = std::shared_ptr<const Info>;	///< Shared pointer to Info.

			/**
			 * @brief Constructor.
			 * @param name The name of the container.
			 * @param allowedCodecs The list of allowed codecs.
			 * @param maxStreams The maximum number of streams allowed in the container.
			 */
			Info(const Container::Name& name, const std::string& name_string, const std::vector<Codec::Name>& allowedCodecs);

			/**
			 * @brief Constructor.
			 * @param name The name of the container.
			 * @param allowedCodecs The list of allowed codecs.
			 * @param maxStreams The maximum number of streams allowed in the container.
			 */
			Info(Container::Name&& name, std::string&& name_string, std::vector<Codec::Name>&& allowedCodecs);

			/**
			 * @brief Copy constructor.
			 * @param info The Info to copy.
			 */
			Info(const Info& info) 						= default;

			/**
			 * @brief Move constructor.
			 * @param info The Info to move.
			 */
			Info(Info&& info) noexcept 					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param info The Info to copy.
			 * @return The copied Info.
			 */
			Info& operator=(const Info& info)			= default;

			/**
			 * @brief Move assignment operator.
			 * @param info The Info to move.
			 * @return The moved Info.
			 */
			Info& operator=(Info&& info) noexcept 		= default;

			/**
			 * @brief Destructor.
			 */
			~Info() noexcept 							= default;

			/**
			 * @brief Gets container name
			 * @return The container name.
			 */
			const Container::Name& 						Name() const noexcept;

			/**
			 * @brief Gets container name as string
			 * @return The container name.
			 */
			const std::string& 							NameToString() const noexcept;

			/**
			 * @brief Gets all the allowed codecs for the container.
			 * @return The vector of allowed codecs.
			 */
			const std::vector<Codec::Name>&				AllowedCodecs() const noexcept;

		private:
			Container::Name 							m_name;				///< The name of the container (e.g., "MP4").
			std::string 								m_name_string;		///< The name of the container as a string.
			std::vector<Codec::Name> 					m_allowed_codecs;	///< The list of codecs allowed in the container.
	};
}