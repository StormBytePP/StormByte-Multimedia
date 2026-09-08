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

#include <StormByte/multimedia/pipeline/filters/chain/generic.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Chain
 */
namespace StormByte::Multimedia::Pipeline::Filter::Chain {
	/**
	 * @class Process
	 * @brief Decode-to-encode @ref StormByte::Multimedia::Pipeline::Filter::Process list.
	 *
	 * Owned by the pipeline, not by Transcode. Zero nodes: Push is
	 * identity. A node that fails stops the chain;
	 * @ref Generic::Error is that node's message.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Process: public Generic<Filter::Process> {
		public:
			/**
			 * @brief Empty chain.
			 */
			Process() noexcept = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Process(const Process&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Process(Process&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Process() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Process& operator=(const Process&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Process& operator=(Process&&) noexcept = default;

			/**
			 * @brief Runs every node with @ref Role::Process.
			 * @param frame Unit to mutate in place.
			 * @return false if this chain @ref Failed.
			 */
			bool Push(Pipeline::Frame& frame) noexcept;
	};
}
