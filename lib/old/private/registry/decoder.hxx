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
