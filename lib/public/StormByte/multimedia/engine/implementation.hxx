#pragma once

#include <StormByte/multimedia/visibility.h>

#include <unordered_set>

/**
 * @namespace Engine
 * @brief The namespace for all multimedia engine classes.
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @enum Implementation
	 * @brief Enumeration of supported multimedia implementations.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Implementation {
		FFmpeg,															///< FFmpeg multimedia implementation
	};

	/**
	 * @brief Convert Implementation enum to string representation.
	 * @param implementation The implementation enum value.
	 * @return The string representation of the implementation.
	 */
	constexpr const char* 												ToString(Implementation implementation) noexcept {
		switch (implementation) {
			case Implementation::FFmpeg:	return "FFmpeg";
			default:						return "Unknown";
		}
	}

	/**
	 * @brief Get the set of available multimedia implementations.
	 * @return The set of available implementations.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC std::unordered_set<Implementation> 		ImplementedBackends() noexcept;
}