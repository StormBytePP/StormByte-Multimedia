#pragma once

#include <StormByte/alias.hxx>
#include <StormByte/multimedia/exception.hxx>
#include <StormByte/multimedia/media/type.hxx>

#include <string>
#include <unordered_map>
#include <vector>

/**
 * @namespace Codec
 * @brief The namespace for all multimedia media codec information.
 */
namespace StormByte::Multimedia::Media::Codec {
	/**
	 * @enum ID
	 * @brief Enumerates all supported codec names.
	 */
	enum STORMBYTE_MULTIMEDIA_PUBLIC ID: unsigned short {
		// Audio codecs
		AAC,									///< AAC audio codec.
		AC3,									///< AC3 audio codec.
		DTS,									///< DTS audio codec.
		EAC3,									///< Enhanced AC3 audio codec.
		FLAC,									///< FLAC audio codec.
		MP3,									///< MP3 audio codec.
		OPUS,									///< OPUS audio codec.
		PCM,									///< PCM audio codec.
		VORBIS,									///< Vorbis audio codec.
		WMA,									///< Windows Media Audio codec.
		ALAC,									///< Apple Lossless Audio Codec.
		MPEG1L2,								///< MPEG-1 Layer II Audio codec.

		// Video codecs
		AV1,									///< AV1 video codec.
		AVC,									///< AVC (Advanced Video Codec).
		H264,									///< H264 video codec.
		H265,									///< H265 (HEVC) video codec.
		MJPEG,									///< Motion JPEG video codec.
		THEORA,									///< Theora video codec.
		VP8,									///< VP8 video codec.
		VP9,									///< VP9 video codec.
		XVID,									///< Xvid video codec.

		// Subtitle codecs
		ASUB,									///< Advanced SubStation Alpha subtitle codec.
		SUBRIP,									///< SubRip subtitle codec.
		WEBVTT,									///< WebVTT subtitle codec.

		// Image codecs
		BMP,									///< BMP image codec.
		GIF,									///< GIF image codec.
		JPEG,									///< JPEG image codec.
		PNG,									///< PNG image codec.
		TIFF,									///< TIFF image codec.
		JPEG_XL									///< JPEG XL image codec.
	};

	/**
	 * @class Info
	 * @brief Holds detailed information about a codec.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Info {
		public:
			using PointerType = std::shared_ptr<const Info>;	///< Shared pointer to Info.

			/**
			 * @brief Constructor.
			 * @param name The name of the codec.
			 * @param name_string The name of the codec as a string.
			 * @param type The type of the codec.
			 */
			Info(const Codec::ID& name, const Type& type, const std::string& name_string) noexcept;
			
			/**
			 * @brief Constructor.
			 * @param name The name of the codec.
			 * @param name_string The name of the codec as a string.
			 * @param type The type of the codec.
			 */
			Info(Codec::ID&& name, Type&& type, std::string&& name_string) noexcept;

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
			 * @brief Gets the name of the codec.
			 * @return The name of the codec.
			 */
			const Codec::ID& 							ID() const noexcept;

			/**
			 * @brief Gets the name of the codec.
			 * @return The name of the codec.
			 */
			const std::string& 							Name() const noexcept;

			/**
			 * @brief Gets the type of the codec.
			 * @return The type of the codec.
			 */
			const Media::Type&							Type() const noexcept;

		private:
			Codec::ID 									m_name;			///< Name of the codec.
			Media::Type 								m_type;			///< Type of the codec (Audio, Video, Subtitle, or Image).
			std::string 								m_name_string;	///< Name of the codec (e.g., "AAC", "H264").
	};
}
