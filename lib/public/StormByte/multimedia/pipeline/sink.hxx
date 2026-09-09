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

#include <StormByte/multimedia/pipeline/hopper.hxx>
#include <StormByte/multimedia/pipeline/item.hxx>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Route;
	class Router;
	class Step;
	class Transcode;

	STORMBYTE_MULTIMEDIA_PUBLIC Step& operator>>(Step& from, Step& to) noexcept;

	/**
	 * @class Sink
	 * @brief Set of @ref Hopper buckets keyed by origin track.
	 *
	 * Private to the tube (moves to Buffer later). Default is zero
	 * buckets. One bucket is one track or one @ref Kind. N buckets
	 * (Demux out, Mux in) key by origin track.
	 *
	 * Read/write that is not wiring blocks until **that** bucket
	 * exists. @ref Push(int, std::shared_ptr<Item>) waits for
	 * @p track. @ref Pop waits until at least one bucket exists.
	 * @ref Bind / @ref Wake never block. This is the only blocking
	 * Sink does. The consumer Step CV is separate (@ref Wake).
	 *
	 * @ref Push(std::shared_ptr<Item>) forwards to
	 * @ref Push(int, std::shared_ptr<Item>) with @ref Item::Track.
	 *
	 * @ref EoF with zero buckets is false (no tube yet). @ref Eof
	 * marks only buckets that already exist.
	 *
	 * A bucket has no Fail.
	 *
	 * @todo Buffer next release.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class Sink {
		friend class Route;
		friend class Router;
		friend class Step;
		friend class Transcode;
		friend Step& operator>>(Step& from, Step& to) noexcept;

		public:
			/**
			 * @brief Selector for @ref Pop on an N-bucket sink.
			 *
			 * Argument is the number of buckets. Return is an index
			 * in @c [0, count). Transcode passes a function that
			 * prefers encode tracks from the Plan. Default is
			 * round-robin. One-bucket sinks never call it.
			 */
			using Select = std::function<std::size_t(std::size_t)>;

			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Zero buckets. Read/write wait for @ref Bind.
			 */
			Sink() noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source sink.
			 */
			Sink(const Sink& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Sink to take.
			 */
			Sink(Sink&& other) noexcept = delete;

			/**
			 * @brief Destructor. Wakes waiters on @ref m_wired.
			 */
			~Sink() noexcept;

			/**
			 * @brief Copy assignment.
			 * @param other Source sink.
			 * @return *this.
			 */
			Sink& operator=(const Sink& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Sink to take.
			 * @return *this.
			 */
			Sink& operator=(Sink&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @name Producer
			 * @{
			 */

			/**
			 * @brief Pushes @p item into the bucket for @ref Item::Track.
			 * @param item Unit to enqueue. Empty pointers are discarded.
			 *
			 * Forwards to @ref Push(int, std::shared_ptr<Item>).
			 */
			void Push(std::shared_ptr<Item> item) noexcept;

			/**
			 * @brief Pushes @p item into bucket @p track.
			 * @param track Origin stream index.
			 * @param item Unit to enqueue. Empty pointers are discarded.
			 *
			 * Blocks until that bucket exists. Does not block once
			 * the hopper is there.
			 */
			void Push(int track, std::shared_ptr<Item> item) noexcept;

			/**
			 * @brief Marks every bound hopper @ref Hopper::Eof.
			 *
			 * Does not wait for missing tracks. Zero buckets: no-op.
			 */
			void Eof() noexcept;

			/**
			 * @brief Shares every existing hopper with @p consumer.
			 * @param consumer Input sink of the consumer step.
			 *
			 * Does not create buckets. Zero buckets: no-op.
			 */
			void Bind(Sink& consumer);

			/**
			 * @brief Creates the bucket for @p track and shares it.
			 * @param track Origin stream index.
			 * @param consumer Input sink of the consumer step.
			 *
			 * Wakes Push/Pop waiters for that bucket.
			 */
			void Bind(int track, Sink& consumer);

			/**
			 * @}
			 */

			/**
			 * @name Consumer
			 * @{
			 */

			/**
			 * @brief Points every hopper at the consumer step CV.
			 * @param wake Consumer @c Wake() CV. Not owned.
			 */
			void Wake(std::condition_variable& wake) noexcept;

			/**
			 * @brief Pops one item. No track argument.
			 * @return Next pointer, or empty. Empty plus @ref EoF is tube EoF.
			 *
			 * Blocks while there are zero buckets.
			 */
			std::shared_ptr<Item> Pop() noexcept;

			/**
			 * @brief Pops one item with an index selector.
			 * @param select Index chooser. Empty: round-robin.
			 * @return Next pointer, or empty.
			 *
			 * Blocks while there are zero buckets. Not used as a
			 * selector when there is one bucket.
			 */
			std::shared_ptr<Item> Pop(const Select& select) noexcept;

			/**
			 * @brief Whether every existing bucket is @ref Hopper::EoF and empty.
			 * @return false when there are zero buckets. Otherwise true
			 *         when the consumer may treat this sink as finished.
			 *
			 * There is no per-track EoF. Does not pop.
			 */
			bool EoF() const noexcept;

			/**
			 * @}
			 */

		private:
			/**
			 * @brief Whether @ref Step::Wait may return.
			 * @return true if a @ref Pop would see an item, or @ref EoF.
			 *
			 * Zero buckets: false. Does not pop. Friend of @ref Step.
			 */
			bool Ready() const noexcept;

			/**
			 * @brief Creates the hopper for @p track if missing.
			 * @param track Origin stream index.
			 * @return Shared hopper. Caller holds @ref m_mutex.
			 */
			std::shared_ptr<Hopper<std::shared_ptr<Item>>> Ensure(int track);

			/**
			 * @brief Rebuilds the pop order from @ref m_buckets.
			 *
			 * Caller holds @ref m_mutex.
			 */
			void RebuildOrder();

			/**
			 * @brief Snapshot of hoppers in pop order.
			 * @return Shared hoppers. Safe to use without @ref m_mutex.
			 */
			std::vector<std::shared_ptr<Hopper<std::shared_ptr<Item>>>> Order() const;

			mutable std::mutex m_mutex;													///< Guards the map
			std::condition_variable m_wired;											///< Waits for Bind of a bucket
			std::map<int, std::shared_ptr<Hopper<std::shared_ptr<Item>>>> m_buckets;	///< Track -> hopper
			std::vector<std::shared_ptr<Hopper<std::shared_ptr<Item>>>> m_order;			///< Stable pop order
			std::atomic<std::size_t> m_rr;												///< Round-robin cursor
			std::atomic<std::condition_variable*> m_wake;								///< Consumer CV
	};
}
