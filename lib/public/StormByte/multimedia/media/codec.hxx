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
	 * @enum Name
	 * @brief Enumerates all supported codec names.
	 */
	enum STORMBYTE_MULTIMEDIA_PUBLIC Name: unsigned short {
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
	 * @struct CodecInfo
	 * @brief Holds detailed information about a codec.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Info {
		std::string s_name;						///< Name of the codec (e.g., "AAC", "H264").
		Type s_type;							///< Type of the codec (Audio, Video, Subtitle, or Image).
	};

	/**
	 * @class CodecRegistry
	 * @brief Centralized registry to manage codec-related metadata.
	 */
	struct STORMBYTE_MULTIMEDIA_PUBLIC Registry {
		/**
		 * @brief Retrieves detailed information about a codec.
		 * @param codec The codec enum value.
		 * @return A reference to the CodecInfo struct for the requested codec.
		 */
		static const Info& Info(const Name& codec);

		/**
		 * @brief Retrieves detailed information about a codec by name.
		 * @param codecName The name of the codec.
		 * @return A reference to the CodecInfo struct for the requested codec.
		 */
		static StormByte::Expected<Name, CodecNotFound> Info(const std::string& codecName);

		private:
			static const std::unordered_map<Codec::Name, Codec::Info> c_codec_registry; 	///< The codec registry.
			static const std::unordered_map<std::string, Codec::Name> c_codec_name_map;	///< The codec name map.
	};
}
