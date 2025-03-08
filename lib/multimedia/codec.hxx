#pragma once

#include <multimedia/media/types.hxx>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	class STORMBYTE_MULTIMEDIA_PUBLIC Codec {
		public:
			/**
			 * @enum Name
			 * @brief The codec name.
			 */
			enum class Name: unsigned short {
				// Audio codecs
				AC3,		///< AC3 audio codec.
				AAC,		///< AAC audio codec.
				MP3,		///< MP3 audio codec.
				PCM,		///< PCM audio codec.
				EAC3,		///< EAC3 audio codec.
				FLAC,		///< FLAC audio codec.
				OPUS,		///< OPUS audio codec.
				// Video codecs
				H264,		///< H264 video codec.
				VP8,		///< VP8 video codec.
				VP9,		///< VP9 video codec.
				THEORA,		///< Theora video codec.
				// Subtitle codecs
				SUBRIP,		///< SubRip subtitle codec.
}