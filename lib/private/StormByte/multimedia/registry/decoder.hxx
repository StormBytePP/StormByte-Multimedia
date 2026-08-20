#pragma once

#include <StormByte/multimedia/feature.hxx>
#include <StormByte/multimedia/registry/entry/implementation.hxx>

#include <array>

using DecoderEntry = StormByte::Multimedia::Registry::Entry::Implementation::Entry;

/**
 * @namespace Registry
 * @brief Compile-time capability tables for codecs and implementations.
 */
namespace StormByte::Multimedia::Registry {
	/**
	 * @brief Named decoder implementation registry (video + audio).
	 *
	 * Maps FFmpeg decoder names to StormByte Feature flags. Runtime detection
	 * may still add MultiThreaded / HardwareAcceleration from AVCodec caps.
	 */
	STORMBYTE_MULTIMEDIA_PRIVATE constexpr auto Decoder = std::array{
		// ----------------------------------------------------------------
		// H.264 / AVC
		// ----------------------------------------------------------------

		/** @brief Software H.264 decoder */
		DecoderEntry("h264",
			Feature::BFrames |
			Feature::Slices |
			Feature::IntraOnly |
			Feature::Interlaced |
			Feature::TenBit
		),

		/** @brief NVIDIA NVDEC H.264 */
		DecoderEntry("h264_nvdec",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::IntraOnly
		),

		/** @brief Intel QSV H.264 */
		DecoderEntry("h264_qsv",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::IntraOnly
		),

		/** @brief Apple VideoToolbox H.264 */
		DecoderEntry("h264_videotoolbox",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices
		),

		// ----------------------------------------------------------------
		// H.265 / HEVC
		// ----------------------------------------------------------------

		/** @brief Software HEVC decoder */
		DecoderEntry("hevc",
			Feature::BFrames |
			Feature::Slices |
			Feature::IntraOnly |
			Feature::Interlaced |
			Feature::TenBit |
			Feature::TwelveBit |
			Feature::HDR10 |
			Feature::HDR10Plus |
			Feature::WideGamut
		),

		/** @brief NVIDIA NVDEC HEVC */
		DecoderEntry("hevc_nvdec",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::HDR10 |
			Feature::WideGamut
		),

		/** @brief Intel QSV HEVC */
		DecoderEntry("hevc_qsv",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::HDR10
		),

		/** @brief Apple VideoToolbox HEVC */
		DecoderEntry("hevc_videotoolbox",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit
		),

		// ----------------------------------------------------------------
		// VP8 / VP9
		// ----------------------------------------------------------------

		/** @brief libvpx VP8 decoder */
		DecoderEntry("libvpx",
			Feature::LowDelay |
			Feature::Slices
		),

		/** @brief libvpx VP9 decoder */
		DecoderEntry("libvpx-vp9",
			Feature::LowDelay |
			Feature::Slices |
			Feature::TenBit |
			Feature::TwelveBit |
			Feature::HDR10 |
			Feature::WideGamut
		),

		// ----------------------------------------------------------------
		// Audio
		// ----------------------------------------------------------------

		/** @brief Native AAC decoder */
		DecoderEntry("aac",
			Feature::LowDelay
		),

		/** @brief AC-3 / E-AC-3 */
		DecoderEntry("ac3",
			Feature::HighQuality
		),

		/** @brief Fraunhofer FDK AAC */
		DecoderEntry("fdk_aac",
			Feature::HighQuality |
			Feature::LowDelay
		),

		/** @brief Native Vorbis */
		DecoderEntry("vorbis",
			Feature::LowDelay
		),

		/** @brief libvorbis */
		DecoderEntry("libvorbis",
			Feature::HighQuality |
			Feature::LowDelay
		),

		/** @brief Native Opus */
		DecoderEntry("opus",
			Feature::LowDelay
		),

		/** @brief libopus */
		DecoderEntry("libopus",
			Feature::HighQuality |
			Feature::LowDelay
		),

		/** @brief Native MP3 */
		DecoderEntry("mp3",
			Feature::LowDelay
		),

		/** @brief LAME MP3 (as decoder name where applicable) */
		DecoderEntry("libmp3lame",
			Feature::HighQuality |
			Feature::LowDelay
		),

		/** @brief FLAC */
		DecoderEntry("flac",
			Feature::HighQuality |
			Feature::Lossless
		),

		/** @brief DTS / DCA */
		DecoderEntry("dca",
			Feature::HighQuality
		),

		/** @brief TrueHD / MLP */
		DecoderEntry("truehd",
			Feature::HighQuality |
			Feature::Lossless
		),

		/** @brief ALAC */
		DecoderEntry("alac",
			Feature::HighQuality |
			Feature::Lossless
		),
	};
}
