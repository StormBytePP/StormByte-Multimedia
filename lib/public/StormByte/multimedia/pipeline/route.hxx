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

#include <StormByte/multimedia/pipeline/item.hxx>
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
	namespace Filter {
		class FFmpeg;	///< Filter base. Owned by @ref Route.
		class Report;	///< Measurement from one filter.
	}

	/**
	 * @class Route
	 * @brief Glue for one origin track and the filters of that track.
	 *
	 * @ref Add takes ownership of one filter instance. Filters are not
	 * wired by hand. Inherit @ref Filter::Process, @ref Filter::Packet
	 * or @ref Filter::Analytics; a bare @ref Filter::FFmpeg is
	 * @ref Step::Fail at Add.
	 *
	 * For each @ref Kind the route keeps a process chain and an
	 * analytics chain. Analytics of a Kind always sit after the
	 * process nodes of that Kind, even if they were @ref Add 'd first.
	 *
	 * @ref Close is per track: one origin, one destination, one stretch.
	 * O(1) at the ends: origin binds the first filter, the last filter
	 * binds the destination. No filters kept: origin binds destination.
	 * Frame filters on a copy track are ignored; there are no frames.
	 * Packet / BSF filters on copy stay.
	 *
	 * @ref Add calls @ref Step::Launch on the kept filter.
	 * @ref Close only binds; it does not launch.
	 *
	 * @ref Reports is collected at route EoF, one entry per owned
	 * filter, in Add order.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Route {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Route for one origin track.
			 * @param track Origin stream index.
			 * @param copy true if this track is stream-copy.
			 */
			explicit Route(int track, bool copy = false) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source route.
			 */
			Route(const Route& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Route to take.
			 */
			Route(Route&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Route() noexcept;

			/**
			 * @brief Copy assignment.
			 * @param other Source route.
			 * @return *this.
			 */
			Route& operator=(const Route& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Route to take.
			 * @return *this.
			 */
			Route& operator=(Route&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @name Identity
			 * @{
			 */

			/**
			 * @brief Origin track.
			 * @return Stream index passed to the constructor.
			 */
			int Track() const noexcept;

			/**
			 * @brief Whether this track is stream-copy.
			 * @return true if copy.
			 */
			bool Copy() const noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Filters
			 * @{
			 */

			/**
			 * @brief Takes ownership of @p filter, hooks it and launches it.
			 * @param filter Filter instance for this track.
			 *
			 * Frame-only filters on a copy route are dropped.
			 * A leaf that is not Process, Packet or Analytics fails the job.
			 */
			void Add(std::shared_ptr<Filter::FFmpeg> filter) noexcept;

			/**
			 * @brief Wires this track from @p origin to @p destination.
			 * @param origin Producer step.
			 * @param destination Consumer step.
			 *
			 * O(1) at the ends. Uses @c Bind(track, …). Does not launch.
			 */
			void Close(Step& origin, Step& destination) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Reports
			 * @{
			 */

			/**
			 * @brief Reports of every kept filter.
			 * @return One entry per owned filter, in Add order.
			 *
			 * Call at route EoF. Dropped copy-frame filters are absent.
			 */
			std::vector<Filter::Report> Reports() const noexcept;

			/**
			 * @}
			 */

		private:
			/**
			 * @class Lane
			 * @brief Process chain and analytics chain of one @ref Kind.
			 */
			struct STORMBYTE_MULTIMEDIA_PRIVATE Lane {
				Filter::FFmpeg* FirstProcess = nullptr;		///< Head of process
				Filter::FFmpeg* LastProcess = nullptr;		///< Tail of process
				Filter::FFmpeg* FirstAnalytics = nullptr;	///< Head of analytics
				Filter::FFmpeg* LastAnalytics = nullptr;	///< Tail of analytics

				/**
				 * @brief First node of this lane.
				 * @return Process head, else analytics head, else null.
				 */
				Filter::FFmpeg* First() const noexcept {
					return FirstProcess != nullptr ? FirstProcess : FirstAnalytics;
				}

				/**
				 * @brief Last node of this lane.
				 * @return Analytics tail, else process tail, else null.
				 */
				Filter::FFmpeg* Last() const noexcept {
					return LastAnalytics != nullptr ? LastAnalytics : LastProcess;
				}
			};

			/**
			 * @brief Hooks @p filter onto @p lane in O(1).
			 * @param lane Process / analytics of one Kind.
			 * @param filter Node to append.
			 * @param analytics true if @p filter is Analytics.
			 */
			void Hook(Lane& lane, Filter::FFmpeg& filter, bool analytics) noexcept;

			int m_track;											///< Origin stream index
			bool m_copy;											///< Stream-copy track
			Lane m_frames;											///< @ref Kind::Frame
			Lane m_packets;											///< @ref Kind::Packet
			std::vector<std::shared_ptr<Filter::FFmpeg>> m_filters;	///< Owned filters, Add order
	};
}
