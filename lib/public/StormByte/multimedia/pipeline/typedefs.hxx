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

#include <StormByte/bitmask.hxx>
#include <StormByte/expected.hxx>
#include <StormByte/multimedia/pipeline/exception.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	using CheckResult = StormByte::Expected<void, PlanException>;	///< Outcome of @ref Plan::Check

	/**
	 * @enum Kind
	 * @brief Whether an @ref Item is a decoded frame or a compressed packet.
	 *
	 * Distinct from @ref StormByte::Multimedia::Type (Video / Audio / Subtitle).
	 * Values are bit flags. @c 0 is not a valid kind. Combine with @ref Kinds.
	 * @ref ToString understands a single flag only.
	 *
	 * @ingroup multimedia_pipeline
	 */
	enum class Kind: std::uint8_t {
		Packet = 1 << 0,	///< @ref StormByte::Multimedia::Pipeline::Packet
		Frame  = 1 << 1		///< @ref StormByte::Multimedia::Pipeline::Frame
	};

	/**
	 * @brief Converts a single @ref Kind flag to a string literal.
	 * @param kind Value to convert.
	 * @return `"Frame"`, `"Packet"`, or `"Invalid"` for a mask or zero.
	 */
	constexpr const char* ToString(Kind kind) noexcept {
		switch (kind) {
			case Kind::Frame:	return "Frame";
			case Kind::Packet:	return "Packet";
			default:			return "Invalid";
		}
	}

	/**
	 * @class Kinds
	 * @brief Bitmask of @ref Kind.
	 *
	 * Every @ref Step stores one mask as Receives and one as Produces.
	 * Empty means that side does not take or emit items (Demux receives
	 * nothing, Mux produces nothing).
	 *
	 * Tests use @ref StormByte::Bitmask::Has (all bits) and
	 * @ref StormByte::Bitmask::HasAny.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Kinds: public StormByte::Bitmask<Kinds, Kind> {
		public:
			using StormByte::Bitmask<Kinds, Kind>::Bitmask;
	};
}
