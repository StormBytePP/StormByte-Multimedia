/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia is dual-licensed under the following terms:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You can redistribute it and/or modify it under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this software may be used under the terms of a
 *    commercial license agreement with the sole copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *    Contact the copyright holder for more information.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this StormByte-Multimedia. If not, see <https://www.gnu.org/licenses/>.
 */

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
