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

#include <StormByte/multimedia/visibility.h>

#include <cstddef>

#if defined(__cpp_lib_reflection)
#	include <meta>
#endif

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter
 * @brief Facades, chain and reports for pipeline filter plugins.
 */
namespace StormByte::Multimedia::Pipeline::Filter {
	/**
	 * @enum Origin
	 * @brief Stage that is calling a filter.
	 *
	 * Powers of two, no holes, start at `1u << 0`. Used as flags and
	 * as index (`countr_zero`) on @ref Chain.
	 */
	enum class Origin : unsigned short {
		Demux   = 1u << 0,	///< Demuxer
		Decoder = 1u << 1,	///< Decoder
		Encoder = 1u << 2,	///< Encoder
		Mux     = 1u << 3	///< Muxer
	};

	/**
	 * @brief Number of @ref Origin enumerators.
	 */
#if defined(__cpp_lib_reflection)
	inline constexpr std::size_t OriginCount =
		std::meta::enumerators_of(^^Origin).size();
#else
	inline constexpr std::size_t OriginCount = 4;
#endif
}
