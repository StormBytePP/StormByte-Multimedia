/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/multimedia/feature.hxx>
#include <StormByte/multimedia/registry/entry/codec.hxx>
#include <StormByte/multimedia/type.hxx>

#include <array>

using CodecEntry = StormByte::Multimedia::Registry::Entry::Codec::Entry;

/**
 * @namespace Registry
 * @brief Compile-time capability tables for codecs and implementations.
 */
namespace StormByte::Multimedia::Registry {
	/**
	 * @brief Logical codec registry (video + audio).
	 *
	 * Encodes capabilities as understood by StormByte (not a 1:1 dump of FFmpeg).
	 * Used by Codec::Find(Type, Features) to pick a matching codec id.
	 */
	STORMBYTE_MULTIMEDIA_PRIVATE constexpr auto Codec = std::array{
		// ----------------------------------------------------------------
		// Video
		// ----------------------------------------------------------------

		/** @brief H.264 / AVC */
		CodecEntry{
			AV_CODEC_ID_H264,
			Type::Video,
			Feature::BFrames |
			Feature::Slices |
			Feature::ProfileBased
		},

		/** @brief H.265 / HEVC */
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

		/** @brief AV1 */
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

		// ----------------------------------------------------------------
		// Audio
		// ----------------------------------------------------------------

		/** @brief AAC */
		CodecEntry{
			AV_CODEC_ID_AAC,
			Type::Audio,
			Feature::SurroundSound |
			Feature::ProfileBased
		},

		/** @brief Vorbis */
		CodecEntry{
			AV_CODEC_ID_VORBIS,
			Type::Audio,
			Feature::SurroundSound
		},

		/** @brief Opus */
		CodecEntry{
			AV_CODEC_ID_OPUS,
			Type::Audio,
			Feature::SurroundSound |
			Feature::LowDelay
		},

		/** @brief MP3 */
		CodecEntry{
			AV_CODEC_ID_MP3,
			Type::Audio,
			Feature::SurroundSound
		},

		/** @brief FLAC (lossless) */
		CodecEntry{
			AV_CODEC_ID_FLAC,
			Type::Audio,
			Feature::Lossless |
			Feature::SurroundSound
		},

		/** @brief ALAC (lossless) */
		CodecEntry{
			AV_CODEC_ID_ALAC,
			Type::Audio,
			Feature::Lossless |
			Feature::SurroundSound
		},
	};
}
