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

#include <StormByte/multimedia/pipeline/filters/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter
 * @brief User-supplied packet steps run by Demux.
 */
namespace StormByte::Multimedia::Pipeline::Filter {
	/**
	 * @class Pipe
	 * @brief Ordered list of Filter::Packet steps.
	 *
	 * Zero steps: Push returns the access unit unchanged. An empty
	 * optional from a step stops the chain. A step that Failed()
	 * fails the pipe. Add steps before the first Push.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Pipe: public Packet {
		public:
			/**
			 * @brief Empty pipe (identity).
			 */
			Pipe() noexcept = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Pipe(const Pipe&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Pipe(Pipe&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Pipe() noexcept override = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Pipe& operator=(const Pipe&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Pipe& operator=(Pipe&&) noexcept = default;

			/**
			 * @brief Appends a step. No-op if the pipe already failed.
			 * @param step Owned step (must not be null).
			 */
			void Add(std::unique_ptr<Packet> step) noexcept;

			/**
			 * @brief Number of steps.
			 * @return Count.
			 */
			std::size_t Size() const noexcept;

			/**
			 * @brief Runs every step in order.
			 * @param packet Incoming access unit (moved in).
			 * @return Access unit after the last step, or empty.
			 */
			std::optional<Pipeline::Packet> Push(Pipeline::Packet&& packet) noexcept override;

		private:
			std::vector<std::unique_ptr<Packet>> m_steps;	///< Steps in order
	};
}
