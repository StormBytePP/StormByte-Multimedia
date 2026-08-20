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

#include <StormByte/type_traits.hxx>
#include <StormByte/multimedia/visibility.h>
#include <cstdint>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @enum Feature
	 * @brief Capability flags for codecs / implementations.
	 *
	 * High nibble = category, low nibble = specific feature.
	 */
	enum class Feature : std::uint8_t {
		None = 0x00,

		// PERFORMANCE (0x1x)
		MultiThreaded			= (0x01 << 4) | 0x1,	///< Frame/slice threading
		RealTime				= (0x01 << 4) | 0x2,	///< Suitable for real-time use
		LowDelay				= (0x01 << 4) | 0x3,	///< Low latency
		ZeroCopy				= (0x01 << 4) | 0x4,	///< Zero-copy paths
		HardwareAcceleration	= (0x01 << 4) | 0x5,	///< HW decode/encode

		// QUALITY (0x2x)
		HighQuality				= (0x02 << 4) | 0x1,	///< High quality presets
		PsychoVisual			= (0x02 << 4) | 0x2,	///< Psycho-visual tuning
		AdaptiveQuantization	= (0x02 << 4) | 0x3,	///< Adaptive quantization
		Lookahead				= (0x02 << 4) | 0x4,	///< Lookahead analysis
		TwoPass					= (0x02 << 4) | 0x5,	///< Two-pass encoding
		Lossless				= (0x02 << 4) | 0x6,	///< Lossless mode
		ProfileBased			= (0x02 << 4) | 0x7,	///< Profile selection
		ContentTuning			= (0x02 << 4) | 0x8,	///< Content-type tuning

		// BIT DEPTH / HDR (0x3x)
		TenBit					= (0x03 << 4) | 0x1,	///< 10-bit
		TwelveBit				= (0x03 << 4) | 0x2,	///< 12-bit
		HDR10					= (0x03 << 4) | 0x3,	///< HDR10
		HDR10Plus				= (0x03 << 4) | 0x4,	///< HDR10+
		WideGamut				= (0x03 << 4) | 0x5,	///< Wide color gamut
		SurroundSound			= (0x03 << 4) | 0x6,	///< Surround layouts

		// BITSTREAM STRUCTURE (0x4x)
		BFrames					= (0x04 << 4) | 0x1,	///< B-frames
		IntraOnly				= (0x04 << 4) | 0x2,	///< Intra-only
		Interlaced				= (0x04 << 4) | 0x3,	///< Interlaced
		Slices					= (0x04 << 4) | 0x4,	///< Slice coding

		// OPERATION MODES (0x5x)
		Streamable				= (0x05 << 4) | 0x1,	///< Streamable output
		Encodeable				= (0x05 << 4) | 0x2,	///< Can encode
	};

	/**
	 * Converts a Feature to a human-readable string.
	 * @param feature Feature value.
	 * @return Null-terminated string literal.
	 */
	constexpr const char* ToString(Feature feature) noexcept {
		switch (feature) {
			case Feature::None:					return "None";
			case Feature::MultiThreaded:		return "MultiThreaded";
			case Feature::RealTime:				return "RealTime";
			case Feature::LowDelay:				return "LowDelay";
			case Feature::ZeroCopy:				return "ZeroCopy";
			case Feature::HardwareAcceleration:	return "HardwareAcceleration";
			case Feature::HighQuality:			return "HighQuality";
			case Feature::PsychoVisual:			return "PsychoVisual";
			case Feature::AdaptiveQuantization:	return "AdaptiveQuantization";
			case Feature::Lookahead:			return "Lookahead";
			case Feature::TwoPass:				return "TwoPass";
			case Feature::Lossless:				return "Lossless";
			case Feature::ProfileBased:			return "ProfileBased";
			case Feature::ContentTuning:		return "ContentTuning";
			case Feature::TenBit:				return "TenBit";
			case Feature::TwelveBit:			return "TwelveBit";
			case Feature::HDR10:				return "HDR10";
			case Feature::HDR10Plus:			return "HDR10Plus";
			case Feature::WideGamut:			return "WideGamut";
			case Feature::SurroundSound:		return "SurroundSound";
			case Feature::BFrames:				return "BFrames";
			case Feature::IntraOnly:			return "IntraOnly";
			case Feature::Interlaced:			return "Interlaced";
			case Feature::Slices:				return "Slices";
			case Feature::Streamable:			return "Streamable";
			case Feature::Encodeable:			return "Encodeable";
		}
		return "UnknownFeature";
	}
}
