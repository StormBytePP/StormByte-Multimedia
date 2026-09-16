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

#include <StormByte/multimedia/property/av_rational.hxx>

#include <cstdint>
#include <limits>

/**
 * @namespace StormByte::Multimedia::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::FFmpeg {
	using StormByte::Multimedia::Property::AVRational;	///< Same `{num, den}` the public API uses

	inline constexpr int TimeBase = 1000000;					///< `AV_TIME_BASE` (µs)
	inline constexpr AVRational TimeBaseQ{1, TimeBase};			///< `AV_TIME_BASE_Q`
	inline constexpr AVRational Nanosecond{1, 1000000000};		///< 1 ns tick
	inline constexpr std::int64_t NoPts = std::numeric_limits<std::int64_t>::min();	///< `AV_NOPTS_VALUE`

	/**
	 * @brief `av_rescale_q(ticks, src, dst)`.
	 * @param ticks Source ticks.
	 * @param src Source time base.
	 * @param dst Destination time base.
	 * @return Scaled ticks, or `NoPts` when either side is invalid.
	 */
	inline std::int64_t Rescale(std::int64_t ticks, const AVRational& src, const AVRational& dst) noexcept {
		return src.Rescale(ticks, dst);
	}
}
