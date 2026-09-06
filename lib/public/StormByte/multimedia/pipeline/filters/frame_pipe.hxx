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

#include <StormByte/multimedia/pipeline/filters/frame.hxx>
#include <StormByte/multimedia/visibility.h>

#include <concepts>
#include <memory>
#include <utility>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter
 * @brief Packet and frame steps. Bundled or user-supplied.
 */
namespace StormByte::Multimedia::Pipeline::Filter {
	/**
	 * @class FramePipe
	 * @brief Ordered list of Filter::Step steps.
	 *
	 * Zero steps: Push returns the frame unchanged. An empty optional
	 * from a step stops the chain. A step that Failed() fails the pipe.
	 * Add steps before the first Push.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC FramePipe: public Step {
		public:
			/**
			 * @brief Empty pipe (identity).
			 */
			FramePipe() noexcept = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			FramePipe(const FramePipe&) = delete;

			/**
			 * @brief Move constructor.
			 */
			FramePipe(FramePipe&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~FramePipe() noexcept override = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			FramePipe& operator=(const FramePipe&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			FramePipe& operator=(FramePipe&&) noexcept = default;

			/**
			 * @brief Appends a step. No-op if the pipe already failed.
			 * @param step Owned step (must not be null).
			 */
			void Add(std::unique_ptr<Step> step) noexcept;

			/**
			 * @brief Constructs and appends a step.
			 * @tparam FilterType Type derived from Filter::Step.
			 * @param args Constructor arguments.
			 * @return *this.
			 */
			template<typename FilterType, typename... Args>
			requires std::derived_from<FilterType, Step>
			FramePipe& Add(Args&&... args) noexcept {
				Add(std::make_unique<FilterType>(std::forward<Args>(args)...));
				return *this;
			}

			/**
			 * @brief Number of steps.
			 * @return Count.
			 */
			std::size_t Size() const noexcept;

			/**
			 * @brief Runs every step in order.
			 * @param frame Incoming frame (moved in).
			 * @return Frame after the last step, or empty.
			 */
			std::optional<Pipeline::Frame> Push(Pipeline::Frame&& frame) noexcept override;

		private:
			std::vector<std::unique_ptr<Step>> m_steps;	///< Steps in order
	};
}
