#pragma once

#include <StormByte/multimedia/feature.hxx>
#include <StormByte/multimedia/registry/entry/codec.hxx>
#include <StormByte/multimedia/type.hxx>

#include <array>

using CodecEntry = StormByte::Multimedia::Registry::Entry::Codec::Entry;

/**
 * @namespace Registry
 * @brief The namespace for registering decoders and encoders properties.
 */
namespace StormByte::Multimedia::Registry {
	/**
	 * @brief Codec registry (video + audio).
	 *
	 * This registry encodes *capabilities* of encoders as understood by StormByte,
	 * not just what FFmpeg exposes directly.
	 */
	STORMBYTE_MULTIMEDIA_PRIVATE constexpr auto Codec = std::array{
		// ============================================================
		// VIDEO CODECS
		// ============================================================

		// ============================================================
		// H.264 / AVC
		// ============================================================
		CodecEntry{
			AV_CODEC_ID_H264,
			Type::Video,
			Feature::BFrames |
			Feature::Slices |
			Feature::ProfileBased
		},

		// ============================================================
		// H.265 / HEVC
		// ============================================================
		CodecEntry{
			AV_CODEC_ID_HEVC,
			Type::Video,
			Feature::BFrames |
			Feature::Slices |
			Feature::TenBit |
			Feature::TwelveBit |
			Feature::WideGamut |
			Feature::HDR10 |
			Feature::HDR10Plus |
			Feature::ProfileBased
		},

		// ============================================================
		// AV1
		// ============================================================
		CodecEntry{
			AV_CODEC_ID_AV1,
			Type::Video,
			Feature::BFrames |
			Feature::TenBit |
			Feature::TwelveBit |
			Feature::WideGamut |
			Feature::HDR10 |
			Feature::HDR10Plus |
			Feature::ProfileBased
		},

		// ============================================================
		// AUDIO CODECS
		// ============================================================

		// ============================================================
		// AAC
		// ============================================================
		CodecEntry{
			AV_CODEC_ID_AAC,
			Type::Audio,
			Feature::SurroundSound |
			Feature::ProfileBased
		},

		// ============================================================
		// Vorbis
		// ============================================================
		CodecEntry{
			AV_CODEC_ID_VORBIS,
			Type::Audio,
			Feature::SurroundSound
		},

		// ============================================================
		// Opus
		// ============================================================
		CodecEntry{
			AV_CODEC_ID_OPUS,
			Type::Audio,
			Feature::SurroundSound |
			Feature::LowDelay
		},

		// ============================================================
		// MP3
		// ============================================================
		CodecEntry{
			AV_CODEC_ID_MP3,
			Type::Audio,
			Feature::SurroundSound
		},

		// ============================================================
		// FLAC
		// ============================================================
		CodecEntry{
			AV_CODEC_ID_FLAC,
			Type::Audio,
			Feature::Lossless |
			Feature::SurroundSound
		},

		// ============================================================
		// ALAC
		// ============================================================
		CodecEntry{
			AV_CODEC_ID_ALAC,
			Type::Audio,
			Feature::Lossless |
			Feature::SurroundSound
		},
	};
}
