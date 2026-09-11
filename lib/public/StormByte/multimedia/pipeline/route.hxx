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

#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/filters/report.hxx>
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
	/**
	 * @class Route
	 * @brief Owns the filter chain of one origin track and wires it.
	 *
	 * Packet / BSF filters may sit between Demux and Remux.
	 * Frame / Process filters on that stretch fail at @ref Close.
	 * Encode lanes use Frame and Packet filters between Decoder and Encoder.
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
			 */
			explicit Route(int track) noexcept;

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
			 * @brief Origin track.
			 * @return Stream index passed to the constructor.
			 */
			int Track() const noexcept;

			/**
			 * @brief Takes ownership of @p filter, hooks it and launches it.
			 * @param filter Filter instance for this track.
			 *
			 * A leaf that is not Process, Packet or Analytics fails the filter.
			 */
			void Add(std::shared_ptr<Filter::FFmpeg> filter) noexcept;

			/**
			 * @brief Wires this track from @p origin to @p destination.
			 * @param origin Producer step.
			 * @param destination Consumer step.
			 *
			 * O(1) at the ends. Uses @c Bind(track, …). Does not launch.
			 * If @p destination is a @ref Remux and this route holds a
			 * Frame / Process filter, the remuxer fails.
			 */
			void Close(Step& origin, Step& destination) noexcept;

			/**
			 * @brief Reports of every kept filter.
			 * @return One entry per owned filter, in Add order.
			 */
			std::vector<Filter::Report> Reports() const noexcept;

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
			Lane m_frames;											///< @ref Kind::Frame
			Lane m_packets;											///< @ref Kind::Packet
			std::vector<std::shared_ptr<Filter::FFmpeg>> m_filters;	///< Owned filters, Add order
	};
}
