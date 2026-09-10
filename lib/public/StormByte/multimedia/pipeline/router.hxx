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

#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	/**
	 * @class Router
	 * @brief Wires sinks. Holds no filters, no reports and no progress.
	 *
	 * After @ref Bind the router is disposable. Routes live on the
	 * owner that will call @ref Route::Reports at EoF.
	 *
	 * Copy without packet filters: @c Bind(track, demux, mux), which
	 * is @c from.m_out->Bind(track, to.m_in). Every hopper already on
	 * @p from: @c Bind(demux, mux), which is @c from.m_out->Bind(to.m_in).
	 * A filtered track uses @ref Route::Close instead of this type.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Router {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Empty router.
			 */
			Router() noexcept = default;

			/**
			 * @brief Copy constructor.
			 * @param other Source router.
			 */
			Router(const Router& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Router to take.
			 */
			Router(Router&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Router() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @param other Source router.
			 * @return *this.
			 */
			Router& operator=(const Router& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Router to take.
			 * @return *this.
			 */
			Router& operator=(Router&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @name Bind
			 * @{
			 */

			/**
			 * @brief Shares every hopper already on @p from output with @p to input.
			 * @param from Producer step.
			 * @param to Consumer step.
			 *
			 * Does not create buckets. Zero hoppers: no-op.
			 */
			void Bind(Step& from, Step& to) noexcept;

			/**
			 * @brief Creates the hopper for @p track and shares it.
			 * @param track Origin stream index.
			 * @param from Producer step.
			 * @param to Consumer step.
			 */
			void Bind(int track, Step& from, Step& to) noexcept;

			/**
			 * @}
			 */
	};
}
