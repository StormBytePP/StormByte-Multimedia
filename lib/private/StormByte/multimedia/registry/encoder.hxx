#pragma once

#include <StormByte/multimedia/feature.hxx>
#include <StormByte/multimedia/registry/entry/implementation.hxx>

#include <array>

using EncoderEntry = StormByte::Multimedia::Registry::Entry::Implementation::Entry;

/**
 * @namespace Registry
 * @brief Compile-time capability tables for codecs and implementations.
 */
namespace StormByte::Multimedia::Registry {
	/**
	 * @brief Named encoder implementation registry (video + audio).
	 *
	 * Maps FFmpeg encoder names to StormByte Feature flags. Runtime detection
	 * may still add MultiThreaded / HardwareAcceleration from AVCodec caps.
	 */
	STORMBYTE_MULTIMEDIA_PRIVATE constexpr auto Encoder = std::array{
		// ----------------------------------------------------------------
		// H.264 / AVC
		// ----------------------------------------------------------------

		/** @brief x264 software encoder */
		EncoderEntry("libx264",
			Feature::HighQuality |
			Feature::PsychoVisual |
			Feature::Lookahead |
			Feature::TwoPass |
			Feature::BFrames |
			Feature::Slices |
			Feature::ProfileBased |
			Feature::ContentTuning
		),

		/** @brief NVIDIA NVENC H.264 */
		EncoderEntry("h264_nvenc",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::ProfileBased
		),

		/** @brief Intel QSV H.264 */
		EncoderEntry("h264_qsv",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::ProfileBased
		),

		/** @brief Apple VideoToolbox H.264 */
		EncoderEntry("h264_videotoolbox",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::ProfileBased
		),

		// ----------------------------------------------------------------
		// H.265 / HEVC
		// ----------------------------------------------------------------

		/** @brief x265 software encoder */
		EncoderEntry("libx265",
			Feature::HighQuality |
			Feature::PsychoVisual |
			Feature::Lookahead |
			Feature::TwoPass |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::TwelveBit |
			Feature::HDR10 |
			Feature::HDR10Plus |
			Feature::WideGamut |
			Feature::ProfileBased |
			Feature::ContentTuning
		),

		/** @brief NVIDIA NVENC HEVC */
		EncoderEntry("hevc_nvenc",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::HDR10 |
			Feature::WideGamut |
			Feature::ProfileBased
		),

		/** @brief Intel QSV HEVC */
		EncoderEntry("hevc_qsv",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::HDR10 |
			Feature::ProfileBased
		),

		/** @brief Apple VideoToolbox HEVC */
		EncoderEntry("hevc_videotoolbox",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::ProfileBased
		),

		// ----------------------------------------------------------------
		// AV1
		// ----------------------------------------------------------------

		/** @brief SVT-AV1 software encoder */
		EncoderEntry("svtav1",
			Feature::HighQuality |
			Feature::PsychoVisual |
			Feature::Lookahead |
			Feature::TwoPass |
			Feature::TenBit |
			Feature::WideGamut |
			Feature::ProfileBased
		),

		/** @brief NVIDIA NVENC AV1 */
		EncoderEntry("av1_nvenc",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::TenBit |
			Feature::WideGamut |
			Feature::ProfileBased
		),

		/** @brief Intel QSV AV1 */
		EncoderEntry("av1_qsv",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::TenBit |
			Feature::WideGamut |
			Feature::ProfileBased
		),

		// ----------------------------------------------------------------
		// VP8 / VP9
		// ----------------------------------------------------------------

		/** @brief libvpx VP8 encoder */
		EncoderEntry("libvpx",
			Feature::LowDelay |
			Feature::RealTime |
			Feature::Slices
		),

		/** @brief libvpx VP9 encoder */
		EncoderEntry("libvpx-vp9",
			Feature::LowDelay |
			Feature::RealTime |
			Feature::Slices |
			Feature::TenBit |
			Feature::TwelveBit |
			Feature::HDR10 |
			Feature::WideGamut
		),

		// ----------------------------------------------------------------
		// Audio
		// ----------------------------------------------------------------

		/** @brief Native AAC encoder */
		EncoderEntry("aac",
			Feature::LowDelay
		),

		/** @brief Fraunhofer FDK AAC */
		EncoderEntry("fdk_aac",
			Feature::HighQuality |
			Feature::LowDelay |
			Feature::ProfileBased |
			Feature::SurroundSound
		),

		/** @brief libvorbis */
		EncoderEntry("libvorbis",
			Feature::HighQuality |
			Feature::LowDelay |
			Feature::SurroundSound
		),

		/** @brief libopus */
		EncoderEntry("libopus",
			Feature::HighQuality |
			Feature::LowDelay |
			Feature::SurroundSound
		),

		/** @brief LAME MP3 */
		EncoderEntry("libmp3lame",
			Feature::HighQuality |
			Feature::LowDelay
		),

		/** @brief FLAC */
		EncoderEntry("flac",
			Feature::HighQuality |
			Feature::Lossless |
			Feature::SurroundSound
		),

		/** @brief ALAC */
		EncoderEntry("alac",
			Feature::HighQuality |
			Feature::Lossless |
			Feature::SurroundSound
		),
	};
}
