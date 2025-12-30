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
	 * @brief Enum representing various multimedia features.
	 *
	 * The high 8 bits encode the category.
	 * The low 24 bits encode the specific feature.
	 */
	enum class Feature : std::uint8_t {
		None = 0x00,

		// PERFORMANCE (0x01xxxxxx)
		MultiThreaded						= (0x01 << 4) | 0x1, // 0x11
		RealTime							= (0x01 << 4) | 0x2, // 0x12
		LowDelay							= (0x01 << 4) | 0x3, // 0x13
		ZeroCopy							= (0x01 << 4) | 0x4, // 0x14
		HardwareAcceleration				= (0x01 << 4) | 0x5, // 0x15

		// QUALITY (0x02xxxxxx)
		HighQuality							= (0x02 << 4) | 0x1, // 0x21
		PsychoVisual						= (0x02 << 4) | 0x2, // 0x22
		AdaptiveQuantization 				= (0x02 << 4) | 0x3, // 0x23
		Lookahead							= (0x02 << 4) | 0x4, // 0x24
		TwoPass								= (0x02 << 4) | 0x5, // 0x25
		Lossless							= (0x02 << 4) | 0x6, // 0x26
		ProfileBased						= (0x02 << 4) | 0x7, // 0x27
		ContentTuning						= (0x02 << 4) | 0x8, // 0x28

		// BIT DEPTH / HDR (0x03xxxxxx)
		TenBit								= (0x03 << 4) | 0x1, // 0x31
		TwelveBit							= (0x03 << 4) | 0x2, // 0x32
		HDR10								= (0x03 << 4) | 0x3, // 0x33
		HDR10Plus							= (0x03 << 4) | 0x4, // 0x34
		WideGamut							= (0x03 << 4) | 0x5, // 0x35
		SurroundSound						= (0x03 << 4) | 0x6, // 0x36

		// BITSTREAM STRUCTURE (0x04xxxxxx)
		BFrames								= (0x04 << 4) | 0x1, // 0x41
		IntraOnly							= (0x04 << 4) | 0x2, // 0x42
		Interlaced							= (0x04 << 4) | 0x3, // 0x43
		Slices								= (0x04 << 4) | 0x4, // 0x44

		// OPERATION MODES (0x05xxxxxx)
		Streamable							= (0x05 << 4) | 0x1, // 0x51
		Encodeable							= (0x05 << 4) | 0x2, // 0x52
	};

    /**
     * @brief Convert a Feature enum value to a human-readable string.
     * @param feature The feature to convert.
     * @return A constexpr null-terminated string literal.
     */
    constexpr const char* ToString(Feature feature) noexcept {
        switch (feature) {
            case Feature::None:                 return "None";

            // PERFORMANCE
            case Feature::MultiThreaded:        return "MultiThreaded";
            case Feature::RealTime:             return "RealTime";
            case Feature::LowDelay:             return "LowDelay";
            case Feature::ZeroCopy:             return "ZeroCopy";
            case Feature::HardwareAcceleration: return "HardwareAcceleration";

            // QUALITY
            case Feature::HighQuality:          return "HighQuality";
            case Feature::PsychoVisual:         return "PsychoVisual";
            case Feature::AdaptiveQuantization: return "AdaptiveQuantization";
            case Feature::Lookahead:            return "Lookahead";
            case Feature::TwoPass:              return "TwoPass";
            case Feature::Lossless:             return "Lossless";

            // BIT DEPTH / HDR
            case Feature::TenBit:               return "TenBit";
            case Feature::TwelveBit:            return "TwelveBit";
            case Feature::HDR10:                return "HDR10";
            case Feature::HDR10Plus:            return "HDR10Plus";
            case Feature::WideGamut:            return "WideGamut";

            // BITSTREAM STRUCTURE
            case Feature::BFrames:              return "BFrames";
            case Feature::IntraOnly:            return "IntraOnly";
            case Feature::Interlaced:           return "Interlaced";
            case Feature::Slices:               return "Slices";
        }

        return "UnknownFeature";
    }
}
