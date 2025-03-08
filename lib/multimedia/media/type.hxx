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
		Attachment		///< Attachment stream.
	};

	/**
	 * @enum Codec
	 * @brief The codec name.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Codec : unsigned short {
		// Audio codecs
		AAC,		///< AAC audio codec.
		AC3,		///< AC3 audio codec.
		DTS,		///< DTS audio codec.
		EAC3,		///< EAC3 audio codec.
		FLAC,		///< FLAC audio codec.
		MP3,		///< MP3 audio codec.
		OPUS,		///< OPUS audio codec.
		PCM,		///< PCM audio codec.
		VORBIS,		///< Vorbis audio codec.
		WMA,		///< Windows Media Audio codec.

		// Video codecs
		AV1,		///< AV1 video codec.
		AVC,		///< AVC (Advanced Video Codec).
		H264,		///< H264 video codec.
		H265,		///< H265 (HEVC) video codec.
		MJPEG,		///< Motion JPEG video codec.
		THEORA,		///< Theora video codec.
		VP8,		///< VP8 video codec.
		VP9,		///< VP9 video codec.
		XVID,		///< Xvid video codec.

		// Subtitle codecs
		ASUB,		///< Advanced SubStation Alpha subtitle codec.
		SUBRIP,		///< SubRip subtitle codec.
		WEBVTT,		///< WebVTT subtitle codec.

		// Image codecs
		BMP,		///< BMP image codec.
		GIF,		///< GIF image codec.
		JPEG,		///< JPEG image codec.
		PNG,		///< PNG image codec.
		TIFF 		///< TIFF image codec.
	};	
}