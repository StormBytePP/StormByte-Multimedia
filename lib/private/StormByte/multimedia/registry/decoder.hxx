#pragma once

#include <StormByte/multimedia/feature.hxx>
#include <StormByte/multimedia/registry/entry/implementation.hxx>

#include <array>

using DecoderEntry = StormByte::Multimedia::Registry::Entry::Implementation::Entry;

/**
 * @namespace Registry
 * @brief The namespace for registering decoders and encoders properties.
 */
namespace StormByte::Multimedia::Registry {

	/**
	 * @brief Decoder registry (video + audio).
	 *
	 * This registry encodes *capabilities* of decoders as understood by StormByte,
	 * not just what FFmpeg exposes directly.
	 */
	STORMBYTE_MULTIMEDIA_PRIVATE constexpr auto Decoder = std::array{

		// ============================================================
		// VIDEO DECODERS
		// ============================================================

		// ============================================================
		// H.264 / AVC
		// ============================================================
		DecoderEntry("h264",
			Feature::BFrames |
			Feature::Slices |
			Feature::IntraOnly |
			Feature::Interlaced |
			Feature::TenBit     // High 10 Profile decoding
		),

		DecoderEntry("h264_nvdec",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::IntraOnly
			// No TenBit, no HDR10
		),

		DecoderEntry("h264_qsv",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::IntraOnly
			// No TenBit, no HDR10
		),

		DecoderEntry("h264_videotoolbox",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices
			// No TenBit, no HDR10
		),

		// ============================================================
		// H.265 / HEVC
		// ============================================================
		DecoderEntry("hevc",
			Feature::BFrames |
			Feature::Slices |
			Feature::IntraOnly |
			Feature::Interlaced |
			Feature::TenBit |
			Feature::TwelveBit |
			Feature::HDR10 |
			Feature::HDR10Plus |    // software only
			Feature::WideGamut
		),

		DecoderEntry("hevc_nvdec",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::HDR10 |
			Feature::WideGamut
			// No HDR10Plus
		),

		DecoderEntry("hevc_qsv",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::HDR10
			// No HDR10Plus, no 12-bit
		),

		DecoderEntry("hevc_videotoolbox",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit
			// No HDR10Plus, no 12-bit
		),

		// ============================================================
		// VP8 (libvpx)
		// ============================================================
		DecoderEntry("libvpx",
			Feature::LowDelay |
			Feature::Slices
		),

		// ============================================================
		// VP9 (libvpx-vp9)
		// ============================================================
		DecoderEntry("libvpx-vp9",
			Feature::LowDelay |
			Feature::Slices |
			Feature::TenBit |
			Feature::TwelveBit |
			Feature::HDR10 |
			Feature::WideGamut
		),

		// ============================================================
		// AUDIO DECODERS
		// ============================================================

		// ============================================================
		// AAC
		// ============================================================
		DecoderEntry("aac",
			Feature::LowDelay
		),

		// ============================================================
		// AC3 / E-AC3
		// ============================================================
		DecoderEntry("ac3",
			Feature::HighQuality
		),

		// ============================================================
		// Fraunhofer FDK AAC
		// ============================================================
		DecoderEntry("fdk_aac",
			Feature::HighQuality |
			Feature::LowDelay
		),

		// ============================================================
		// Vorbis
		// ============================================================
		DecoderEntry("vorbis",
			Feature::LowDelay
		),

		DecoderEntry("libvorbis",
			Feature::HighQuality |
			Feature::LowDelay
		),

		// ============================================================
		// Opus
		// ============================================================
		DecoderEntry("opus",
			Feature::LowDelay
		),

		DecoderEntry("libopus",
			Feature::HighQuality |
			Feature::LowDelay
		),

		// ============================================================
		// MP3
		// ============================================================
		DecoderEntry("mp3",
			Feature::LowDelay
		),

		DecoderEntry("libmp3lame",
			Feature::HighQuality |
			Feature::LowDelay
		),

		// ============================================================
		// FLAC
		// ============================================================
		DecoderEntry("flac",
			Feature::HighQuality |
			Feature::Lossless
		),

		// ============================================================
		// DTS / DCA
		// ============================================================
		DecoderEntry("dca",
			Feature::HighQuality
		),
		
		// ============================================================
		// TrueHD / MLP
		// ============================================================
		DecoderEntry("truehd",
			Feature::HighQuality |
			Feature::Lossless
		),

		// ============================================================
		// ALAC
		// ============================================================
		DecoderEntry("alac",
			Feature::HighQuality |
			Feature::Lossless
		),

	};
}
