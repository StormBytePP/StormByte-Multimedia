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

#include <StormByte/multimedia/pipeline/filters/report.hxx>
#include <StormByte/multimedia/pipeline/route.hxx>
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Muxer;

	/**
	 * @class Router
	 * @brief Owns Routes and closes the output graph toward a Muxer.
	 *
	 * Advanced API. Optional: a tube without filters does not need
	 * a Router. If one is constructed, the Muxer is required
	 * (`std::shared_ptr`, same contract as Route ends).
	 *
	 * An empty Muxer pointer is a configuration error and throws
	 * @ref StormByte::Multimedia::Exception. There is no tube that
	 * ends at nullptr.
	 *
	 * Chain @ref Add for each Route, then @ref Close once. Close
	 * requires @ref Muxer::Armed. It does not invent
	 * encoder/remuxer @c operator>> into the Muxer.
	 *
	 * The other Bind overloads stay for hops that have no Route
	 * (unfiltered remux already wired with @c operator>>).
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
			 * @brief Router that closes toward @p muxer.
			 * @param muxer Destination. Must not be empty.
			 * @throws StormByte::Multimedia::Exception if @p muxer is empty.
			 */
			explicit Router(std::shared_ptr<Muxer> muxer);

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

			/**
			 * @name Routes
			 * @{
			 */

			/**
			 * @brief Takes ownership of @p route.
			 * @param route Route to close later. Empty is ignored.
			 * @return *this.
			 */
			Router& Add(std::unique_ptr<Route> route) noexcept;

			/**
			 * @brief Closes the graph: Muxer must be Armed, then each Route.
			 *
			 * Call once, after every @ref Add and after every
			 * encoder/remuxer @c operator>> into the Muxer.
			 *
			 * If the Muxer is not @ref Muxer::Armed, it is Failed.
			 * Route Close still runs so taps are not left half-wired.
			 *
			 * Zero Routes still requires Armed.
			 */
			void Close() noexcept;

			/**
			 * @brief Whether every owned Route is idle.
			 * @return true when every Route reports Idle.
			 */
			bool Idle() const noexcept;

			/**
			 * @brief Reports of every owned Route, in Add order.
			 * @return Concatenated Route reports.
			 */
			std::vector<Filter::Report> Reports() const noexcept;

			/**
			 * @}
			 */

		private:
			std::shared_ptr<Muxer> m_muxer;					///< Destination
			std::vector<std::unique_ptr<Route>> m_routes;	///< Owned tracks
	};
}
