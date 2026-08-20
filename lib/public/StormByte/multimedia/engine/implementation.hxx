#pragma once

#include <StormByte/multimedia/visibility.h>

#include <unordered_set>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @enum Implementation
	 * @brief Available demux/codec backends.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Implementation {
		FFmpeg,	///< FFmpeg-based backend
	};

	/**
	 * Converts Implementation to a string.
	 * @param implementation Backend enum.
	 * @return Null-terminated name.
	 */
	constexpr const char* ToString(Implementation implementation) noexcept {
		switch (implementation) {
			case Implementation::FFmpeg:	return "FFmpeg";
			default:						return "Unknown";
		}
	}

	/**
	 * @return Set of backends compiled into this build.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC std::unordered_set<Implementation> ImplementedBackends() noexcept;
}
