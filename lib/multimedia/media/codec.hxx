#pragma once

#include <multimedia/media/type.hxx>

#include <string>

/**
 * @namespace Media
 * @brief The namespace for all multimedia media codecs.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @enum Codec
	 * @brief The codec name.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Codec: unsigned short {
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
		TIFF, 		///< TIFF image codec.
	};

	/**
	 * @brief Gets the type of the codec.
	 * @param codec The codec.
	 * @return The type of the codec.
	 */
	constexpr Type STORMBYTE_MULTIMEDIA_PUBLIC CodecType(const Codec& codec) noexcept {
		switch (codec) {
			// Audio codecs
			case Codec::AAC:
			case Codec::AC3:
			case Codec::DTS:
			case Codec::EAC3:
			case Codec::FLAC:
			case Codec::MP3:
			case Codec::OPUS:
			case Codec::PCM:
			case Codec::VORBIS:
			case Codec::WMA:
				return Type::Audio;

			// Video codecs
			case Codec::AV1:
			case Codec::AVC:
			case Codec::H264:
			case Codec::H265:
			case Codec::MJPEG:
			case Codec::THEORA:
			case Codec::VP8:
			case Codec::VP9:
			case Codec::XVID:
				return Type::Video;

			// Subtitle codecs
			case Codec::ASUB:
			case Codec::SUBRIP:
			case Codec::WEBVTT:
				return Type::Subtitle;

			// Image codecs
			case Codec::BMP:
			case Codec::GIF:
			case Codec::JPEG:
			case Codec::PNG:
			case Codec::TIFF:
				return Type::Image;

			// Default fallback for unknown codecs
			default:
				return Type::Unknown;
		}
	}

	/**
	 * @brief Gets the name of the codec.
	 * @param codec The codec.
	 * @return The name of the codec.
	 */
	constexpr std::string STORMBYTE_MULTIMEDIA_PUBLIC CodecName(const Codec& codec) noexcept {
		switch (codec) {
			// Audio codecs
			case Codec::AAC:		return "AAC";
			case Codec::AC3:		return "AC3";
			case Codec::DTS:		return "DTS";
			case Codec::EAC3:		return "EAC3";
			case Codec::FLAC:		return "FLAC";
			case Codec::MP3:		return "MP3";
			case Codec::OPUS:		return "OPUS";
			case Codec::PCM:		return "PCM";
			case Codec::VORBIS:		return "VORBIS";
			case Codec::WMA:		return "WMA";
	
			// Video codecs
			case Codec::AV1:		return "AV1";
			case Codec::AVC:		return "AVC";
			case Codec::H264:		return "H264";
			case Codec::H265:		return "H265";
			case Codec::MJPEG:		return "MJPEG";
			case Codec::THEORA:		return "THEORA";
			case Codec::VP8:		return "VP8";
			case Codec::VP9:		return "VP9";
			case Codec::XVID:		return "XVID";
	
			// Subtitle codecs
			case Codec::ASUB:		return "ASUB";
			case Codec::SUBRIP:		return "SUBRIP";
			case Codec::WEBVTT:		return "WEBVTT";
	
			// Image codecs
			case Codec::BMP:		return "BMP";
			case Codec::GIF:		return "GIF";
			case Codec::JPEG:		return "JPEG";
			case Codec::PNG:		return "PNG";
			case Codec::TIFF:		return "TIFF";
			default:				return "Unknown";
		}
	}

	/**
	 * @brief Gets the codec by its name.
	 * @param name The name of the codec.
	 * @throws CodecNotFound If the codec is not found.
	 * @return The codec.
	 */
	Codec STORMBYTE_MULTIMEDIA_PUBLIC CodecByName(const std::string& name);
}