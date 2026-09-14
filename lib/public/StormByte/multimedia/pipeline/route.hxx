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
	 * Packet / BSF filters may sit between Demuxer and Remuxer.
	 * Frame / Process filters on that stretch fail at @ref Close.
	 * Encode lanes use Frame and Packet filters between Decoder and Encoder.
	 *
	 * Analytics is not a process node. @ref Close wires Process from
	 * origin to destination. Frame origins bind @c origin.m_tap onto
	 * the first Analytics (decode look; a deep copy from
	 * @ref Step::Emit). Packet origins (Demuxer on a remux stretch)
	 * spawn a Route-owned Decoder on that tap so Analytics still
	 * sees frames with @ref Producer::Decoder. Destination packet
	 * producers (Encoder or Remuxer) expose @ref Step::Look; Route
	 * builds an encode-look Decoder that stamps
	 * @ref Producer::Encoder and Binds onto the same Analytics
	 * hopper. The last Analytics @c m_out is
	 * @c StormByte::Buffer::Sink::Drain.
	 * A raw tube and Transcoder use this same Close.
	 *
	 * Bind the tap before the origin emits. A late Bind misses every
	 * unit already dropped by Drain on @c m_tap.
	 *
	 * Muxer closed is not the end of this Route. The dest look
	 * lags. @ref Reports before @ref Idle is true has no pooled
	 * Analytics mean. A hand-built tube must wait @ref Idle (or
	 * destroy the Route) before reading Reports. Transcoder waits
	 * the same way before OnDone.
	 *
	 * Analytics that pops and clones into its own queues must bound
	 * those queues itself. @c InputCeiling on the first Analytics
	 * applies to the shared tap hopper after Bind.
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
			 * @brief Destructor. Halts owned filters and look Decoders.
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
			 * Launch starts the worker; the worker waits on @c m_in until
			 * @ref Close binds a hopper. Do not let the origin Emit before Close.
			 */
			void Add(std::shared_ptr<Filter::FFmpeg> filter) noexcept;

			/**
			 * @brief Wires this track from @p origin to @p destination.
			 * @param origin Producer step.
			 * @param destination Consumer step.
			 *
			 * O(1) at the ends. Uses @c Bind(track, …) for Process.
			 * Analytics receives a source look (origin frames via
			 * @c m_tap, or a Decoder on origin packets) and a dest
			 * look via @p destination.Look. Last Analytics @c m_out
			 * is Drained. Does not launch the look Decoder beyond
			 * its own constructor.
			 */
			void Close(Step& origin, Step& destination) noexcept;

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
			 * Snapshots current leaf state. Analytics that has not
			 * run Eof yet returns Failed / empty scores even if
			 * pairs already went to the metric. Call after @ref Idle.
			 */
			std::vector<Filter::Report> Reports() const noexcept;

		private:
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
			 * share @c origin.m_out.
			 */
			void TapDecode(Step& origin, Lane& lane) noexcept;

			/**
			 * @brief Feeds Analytics with the destination look.
			 * @param destination Encoder or Remuxer (packet producer).
			 * @param lane Lane whose FirstAnalytics receives look frames.
			 *
			 * Builds an encode-look Decoder (@ref Producer::Encoder
			 * frames). Calls @p destination.Look. Shares the existing
			 * Analytics hopper onto the look output so a second Bind
			 * does not replace the source tap.
			 */
			void TapEncode(Step& destination, Lane& lane) noexcept;

			int m_track;													///< Origin stream index
			Lane m_frames;													///< Frame process + analytics
			Lane m_packets;													///< Packet process + analytics
			std::vector<std::shared_ptr<Filter::FFmpeg>> m_filters;			///< Owned leaves, Add order
			std::vector<std::unique_ptr<Decoder>> m_looks;					///< Route-owned source and dest look decoders
	};
}
