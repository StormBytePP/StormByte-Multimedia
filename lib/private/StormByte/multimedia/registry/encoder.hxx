#pragma once

#include <StormByte/multimedia/feature.hxx>
#include <StormByte/multimedia/registry/entry/implementation.hxx>

#include <array>

using EncoderEntry = StormByte::Multimedia::Registry::Entry::Implementation::Entry;

/**
 * @namespace Registry
 * @brief The namespace for registering decoders and encoders properties.
 */
namespace StormByte::Multimedia::Registry {

	/**
	 * @brief Encoder registry (video + audio).
	 *
	 * This registry encodes *capabilities* of encoders as understood by StormByte,
	 * not just what FFmpeg exposes directly.
	 */
	STORMBYTE_MULTIMEDIA_PRIVATE constexpr auto Encoder = std::array{

		// ============================================================
		// VIDEO ENCODERS
		// ============================================================

		// ============================================================
		// H.264 / AVC
		// ============================================================
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

		EncoderEntry("h264_nvenc",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |           // NVENC supports 10-bit for H.264
			Feature::ProfileBased
		),

		EncoderEntry("h264_qsv",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::ProfileBased
		),

		EncoderEntry("h264_videotoolbox",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::ProfileBased
		),

		// ============================================================
		// H.265 / HEVC
		// ============================================================
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

		EncoderEntry("hevc_videotoolbox",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::ProfileBased
		),

		// ============================================================
		// AV1
		// ============================================================
		EncoderEntry("svtav1",
			Feature::HighQuality |
			Feature::PsychoVisual |
			Feature::Lookahead |
			Feature::TwoPass |
			Feature::TenBit |
			Feature::WideGamut |
			Feature::ProfileBased
		),

		EncoderEntry("av1_nvenc",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::TenBit |
			Feature::WideGamut |
			Feature::ProfileBased
		),

		EncoderEntry("av1_qsv",
			Feature::HardwareAcceleration |
			Feature::LowDelay |
			Feature::RealTime |
			Feature::TenBit |
			Feature::WideGamut |
			Feature::ProfileBased
		),

		// ============================================================
		// VP8 (libvpx) — Encoder
		// ============================================================
		EncoderEntry("libvpx",
			Feature::LowDelay |
			Feature::RealTime |
			Feature::Slices
		),

		// ============================================================
		// VP9 (libvpx-vp9) — Encoder
		// ============================================================
		EncoderEntry("libvpx-vp9",
			Feature::LowDelay |
			Feature::RealTime |
			Feature::Slices |
			Feature::TenBit |
			Feature::TwelveBit |
			Feature::HDR10 |
			Feature::WideGamut
		),

		// ============================================================
		// AUDIO ENCODERS
		// ============================================================

		// ============================================================
		// AAC
		// ============================================================
		EncoderEntry("aac",
			Feature::LowDelay
		),

		EncoderEntry("fdk_aac",
			Feature::HighQuality |
			Feature::LowDelay |
			Feature::ProfileBased |      // LC, HE, HEv2, LD, ELD
			Feature::SurroundSound
		),

		// ============================================================
		// Vorbis
		// ============================================================
		EncoderEntry("libvorbis",
			Feature::HighQuality |
			Feature::LowDelay |
			Feature::SurroundSound
		),

		// ============================================================
		// Opus
		// ============================================================
		EncoderEntry("libopus",
			Feature::HighQuality |
			Feature::LowDelay |
			Feature::SurroundSound
		),

		// ============================================================
		// MP3
		// ============================================================
		EncoderEntry("libmp3lame",
			Feature::HighQuality |
			Feature::LowDelay
		),

		// ============================================================
		// FLAC
		// ============================================================
		EncoderEntry("flac",
			Feature::HighQuality |
			Feature::Lossless |
			Feature::SurroundSound
		),

		// ============================================================
		// ALAC
		// ============================================================
		EncoderEntry("alac",
			Feature::HighQuality |
			Feature::Lossless |
			Feature::SurroundSound
		),
	};
}
