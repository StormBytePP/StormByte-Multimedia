/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-Multimedia source in this
 * file. Third-party components — including FFmpeg and embedded trained data —
 * remain under their own licenses and are not covered by the commercial grant.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-Multimedia. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
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
