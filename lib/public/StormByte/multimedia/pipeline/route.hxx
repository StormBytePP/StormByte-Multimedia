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

#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/filters/report.hxx>
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <utility>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Router;

	/**
	 * @class Route
	 * @brief Owns the filter chain of one origin track and two ends.
	 *
	 * Advanced API. Construct origin and destination as
	 * @c std::shared_ptr<Step>. Multimedia recommends
	 * @c std::make_shared so the last owner destroys the Step.
	 *
	 * Packet / BSF filters may sit between Demuxer and Remuxer.
	 * Frame / Process filters on that stretch fail at Close.
	 * Encode lanes use Frame and Packet filters between Decoder
	 * and Encoder.
	 *
	 * Analytics is not a process node. Close wires Process from
	 * origin to destination. Frame origins bind @c origin.m_tap
	 * onto the first Analytics (decode look; a deep copy from
	 * @ref Step::Emit). Packet origins spawn a Route-owned
	 * Decoder so Analytics still sees frames with
	 * @ref Producer::Decoder. Destination packet producers
	 * expose @ref Step::Look. The last Analytics Pipe Out is
	 * @c StormByte::Buffer::Sink::Drain.
	 *
	 * Close is private: add Routes to a @ref Router and call
	 * @ref Router::Close. Bind the tap before the origin emits.
	 *
	 * Muxer closed is not the end of this Route. Wait @ref Idle
	 * before @ref Reports.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Route {
		friend class Router;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Route for one origin track and its two ends.
			 * @param track Origin stream index.
			 * @param origin Producer (Decoder / Demuxer). Must not be empty.
			 * @param destination Consumer (Encoder / Remuxer / Muxer). Must not be empty.
			 *
			 * Both ends are real @c shared_ptr. The Step dies when
			 * the last Route, Transcoder lane or caller drops it.
			 */
			Route(int track,
				std::shared_ptr<Step> origin,
				std::shared_ptr<Step> destination) noexcept;

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
			 * @brief Destructor. Halts owned filters and look Decoders
			 *        while those objects are still complete.
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
			 * @param filter Process, Packet or Analytics leaf.
			 * @return *this.
			 *
			 * A leaf that is not Process, Packet or Analytics fails
			 * the filter. Launch starts the worker; the worker waits
			 * on the Pipe until @ref Router::Close binds a hopper.
			 */
			Route& Add(std::shared_ptr<Filter::FFmpeg> filter) noexcept;

			/**
			 * @brief Constructs a leaf of type @p T on this track.
			 * @tparam T Process, Packet or Analytics leaf.
			 * @tparam Args Constructor arguments.
			 * @param args Forwarded to @p T.
			 * @return *this.
			 */
			template<typename T, typename... Args>
			Route& Add(Args&&... args) noexcept {
				return Add(std::shared_ptr<Filter::FFmpeg>(
					std::make_shared<T>(std::forward<Args>(args)...)));
			}

			/**
			 * @brief Whether every owned worker has left Ready.
			 * @return true when all filters and look Decoders are
			 *         Stopped or Failed.
			 *
			 * False while Analytics is still pairing looks. Wait
			 * this before @ref Reports on a hand-built tube.
			 */
			bool Idle() const noexcept;

			/**
			 * @brief Reports of every kept filter.
			 * @return One entry per owned filter, in Add order.
			 *
			 * Call after @ref Idle.
			 */
			std::vector<Filter::Report> Reports() const noexcept;

		private:
			/**
			 * @brief Wires this track from the stored origin to destination.
			 *
			 * Private: @ref Router is the only caller. O(1) at the
			 * ends. Process uses Pipe @c >> . Analytics
			 * receives a source look and a dest look via
			 * destination.Look. Last Analytics Pipe Out is Drained.
			 */
			void Close() noexcept;

			/**
			 * @class Lane
			 * @brief Process chain and analytics chain of one @ref Kind.
			 */
			struct STORMBYTE_MULTIMEDIA_PRIVATE Lane {
				Filter::FFmpeg* FirstProcess = nullptr;
				Filter::FFmpeg* LastProcess = nullptr;
				Filter::FFmpeg* FirstAnalytics = nullptr;
				Filter::FFmpeg* LastAnalytics = nullptr;

				/**
				 * @brief First process node of this lane.
				 * @return Process head, or null. Analytics is not in the tube.
				 */
				Filter::FFmpeg* First() const noexcept {
					return FirstProcess;
				}

				/**
				 * @brief Last process node of this lane.
				 * @return Process tail, or null. Analytics is not in the tube.
				 */
				Filter::FFmpeg* Last() const noexcept {
					return LastProcess;
				}
			};

			/**
			 * @brief Hooks @p filter onto @p lane in O(1).
			 * @param lane Process / analytics of one Kind.
			 * @param filter Node to append.
			 * @param analytics true if @p filter is Analytics.
			 *
			 * Analytics nodes chain to each other only. They do
			 * not Bind after LastProcess.
			 */
			void Hook(Lane& lane, Filter::FFmpeg& filter, bool analytics) noexcept;

			/**
			 * @brief Feeds Analytics with the source look.
			 * @param origin Producer. Must not have started Emit.
			 * @param lane Lane whose FirstAnalytics is the consumer.
			 *
			 * No-op when the lane has no Analytics. Frame producers
			 * bind @c origin.m_tap. Packet producers spawn a Decoder
			 * on that tap (@ref Producer::Decoder frames). Does not
			 * share the origin Pipe Out.
			 */
			void TapDecode(Step& origin, Lane& lane) noexcept;

			/**
			 * @brief Feeds Analytics with the destination look.
			 * @param destination Encoder or Remuxer (packet producer).
			 * @param lane Lane whose FirstAnalytics receives look frames.
			 *
			 * Builds an encode-look Decoder (@ref Producer::Encoder
			 * or @ref Producer::Remuxer frames). Calls
			 * @p destination.Look. Shares the existing Analytics
			 * hopper onto the look output so a second Bind does
			 * not replace the source tap.
			 */
			void TapEncode(Step& destination, Lane& lane) noexcept;

			int m_track;													///< Origin stream index
			std::shared_ptr<Step> m_origin;									///< Producer end
			std::shared_ptr<Step> m_destination;								///< Consumer end
			Lane m_frames;													///< Frame process + analytics
			Lane m_packets;													///< Packet process + analytics
			std::vector<std::shared_ptr<Filter::FFmpeg>> m_filters;			///< Owned leaves, Add order
			std::vector<std::unique_ptr<Decoder>> m_looks;					///< Route-owned source and dest look decoders
	};
}
