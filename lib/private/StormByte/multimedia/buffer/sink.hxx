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

#include <StormByte/multimedia/buffer/hopper.hxx>
#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/visibility.h>

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
 * Forward declarations only in this header. Full documentation lives
 * in the pipeline headers that define the types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Item;
	class Route;
	class Router;
	class Step;
	class Transcoder;

	Step& operator>>(Step& from, Step& to) noexcept;
}

/**
 * @namespace StormByte::Multimedia::Buffer
 * @brief Private tube queues.
 *
 * Hopper is one SPSC bucket. Sink maps integer keys to hoppers.
 * Not a media source; that Buffer is a later type.
 *
 * @ingroup buffer
 */
namespace StormByte::Multimedia::Buffer {
	/**
	 * @class Sink
	 * @brief Set of Hopper buckets keyed by an integer.
	 *
	 * Visibility @c STORMBYTE_MULTIMEDIA_PRIVATE. Not installed.
	 *
	 * Starts with zero buckets. Bind creates or shares a hopper
	 * under @p key. The tube uses Pipeline::Item::Track as the key.
	 *
	 * Tee copies each Push of a key into a new hopper on another
	 * Sink. Bind shares one hopper; Tee never shares. Capacity
	 * applies only to the main bucket. Taps are unbounded and use
	 * Hopper::PushUncapped so Analytics cannot stall the tube.
	 * Eof marks main hoppers and every tap owned by this Sink.
	 *
	 * Push(key) waits on @c m_wired until that bucket exists, the
	 * Sink is closed, or @ref Drain is set. A closed or drained
	 * Sink with no bucket for that key discards the push.
	 * Hopper::Push still waits if Capacity is set and the bucket
	 * is Full.
	 *
	 * Drain is not Eof. Bind after Drain still creates a hopper;
	 * later Push of that key enqueues. Use Drain on a terminal
	 * producer (last Analytics) so Push does not wait for a
	 * consumer that will never Bind.
	 *
	 * Pop waits on @c m_wired until at least one bucket exists or
	 * the Sink is closed. Bind, Tee, Drain and Notify never block.
	 *
	 * Eof closes the Sink even with zero buckets, marks every
	 * existing hopper, and wakes Bind waiters. A Bind after Eof
	 * creates an already-Eof hopper.
	 *
	 * EoF with zero buckets is true only when the Sink is closed.
	 *
	 * A bucket has no Fail. Capacity / Size / Full for a missing
	 * key are noop / 0 / false. Those three look at main buckets
	 * only, not taps.
	 *
	 * Copy and move are deleted (mutex and wired CV).
	 *
	 * @ingroup buffer
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Sink {
		friend class StormByte::Multimedia::Pipeline::Route;
		friend class StormByte::Multimedia::Pipeline::Router;
		friend class StormByte::Multimedia::Pipeline::Step;
		friend class StormByte::Multimedia::Pipeline::Transcoder;
		friend StormByte::Multimedia::Pipeline::Step&
			StormByte::Multimedia::Pipeline::operator>>(
				StormByte::Multimedia::Pipeline::Step& from,
				StormByte::Multimedia::Pipeline::Step& to) noexcept;

		public:
			/**
			 * @brief Chooser for Pop on an N-bucket sink.
			 *
			 * Argument is the number of buckets. Return is an index
			 * in [0, count). Default is round-robin. One-bucket
			 * sinks never call it.
			 */
			using Select = std::function<std::size_t(std::size_t)>;

			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Zero buckets. Read/write wait for Bind or Eof.
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
			 * @brief Destructor. Wakes waiters on m_wired.
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
			 * @brief Pushes item into the bucket for item->Track().
			 * @param item Unit to enqueue. Empty pointers are discarded.
			 */
			void Push(std::shared_ptr<StormByte::Multimedia::Pipeline::Item> item) noexcept;

			/**
			 * @brief Pushes item into bucket key.
			 * @param key Bucket key.
			 * @param item Unit to enqueue. Empty pointers are discarded.
			 *
			 * Waits until the bucket exists, the Sink is closed, or
			 * @ref Drain is set. Closed or drained and no bucket:
			 * discards @p item. Then copies @p item to every Tee of
			 * this key (uncapped).
			 */
			void Push(int key, std::shared_ptr<StormByte::Multimedia::Pipeline::Item> item) noexcept;

			/**
			 * @brief Closes the Sink and marks every hopper Hopper::Eof.
			 *
			 * Wakes Push/Pop waiters on m_wired. Zero buckets still
			 * close the Sink. Marks main hoppers and every tap.
			 */
			void Eof() noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Wiring
			 * @{
			 */

			/**
			 * @brief Shares every existing hopper with consumer.
			 * @param consumer Input sink of the consumer.
			 *
			 * Does not create buckets. Zero buckets: no-op. A key
			 * that already has a Tee to @p consumer is skipped.
			 */
			void Bind(Sink& consumer);

			/**
			 * @brief Creates the bucket for key and shares it.
			 * @param key Bucket key.
			 * @param consumer Input sink of the consumer.
			 *
			 * Wakes Push/Pop waiters for that bucket. If either
			 * Sink is already closed, the hopper is Eof. Hopper
			 * starts unbounded; call Capacity(key, n) after Bind
			 * if needed. No-op if this pair+key is already a Tee.
			 */
			void Bind(int key, Sink& consumer);

			/**
			 * @brief Copies each Push of @p key into a new hopper on @p consumer.
			 * @param key Bucket key (track).
			 * @param consumer Input sink of the tap.
			 *
			 * Does not share the main hopper. Bind already on this
			 * pair+key: no-op. Tee already on this pair+key: no-op.
			 * A second Tee from another producer onto the same
			 * consumer appends another hopper to that consumer's
			 * pop order.
			 *
			 * Tap hoppers are unbounded. Capacity on the main
			 * bucket still applies to the tube Push.
			 */
			void Tee(int key, Sink& consumer) noexcept;

			/**
			 * @brief Tee every bucket already on this Sink.
			 * @param consumer Input sink of the tap.
			 *
			 * Zero buckets: no-op. A later Bind of a new key does
			 * not auto-tee; call Tee(key, consumer) again.
			 */
			void Tee(Sink& consumer) noexcept;

			/**
			 * @brief Terminal producer: Push without a bucket is drop.
			 *
			 * Does not close the Sink and does not mark hoppers Eof.
			 * Bind after Drain still creates a hopper; Push of that
			 * key then enqueues. Wakes Push waiters so a Push that
			 * arrived before Drain can discard.
			 *
			 * The tube must not Drain a Sink that still waits for
			 * Bind (Encoder, Muxer). Last Analytics may: nobody
			 * will Bind its m_out.
			 */
			void Drain() noexcept;

			/**
			 * @brief Whether @ref Drain was called.
			 * @return true after Drain.
			 */
			bool Draining() const noexcept;

			/**
			 * @brief Points every hopper at the consumer CV.
			 * @param consumer Consumer waiter CV. Not owned.
			 */
			void Notify(std::condition_variable& consumer) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Capacity
			 * @{
			 */

			/**
			 * @brief Ceiling of bucket key.
			 * @param key Bucket key.
			 * @return Hopper::Capacity, or 0 if the bucket does not exist.
			 */
			std::size_t Capacity(int key) const noexcept;

			/**
			 * @brief Sets the ceiling of bucket key.
			 * @param key Bucket key.
			 * @param capacity Max items. 0 is unbounded.
			 *
			 * No-op if the bucket does not exist. Does not change taps.
			 */
			void Capacity(int key, std::size_t capacity) noexcept;

			/**
			 * @brief Items waiting in bucket key.
			 * @param key Bucket key.
			 * @return Hopper::Size, or 0 if the bucket does not exist.
			 */
			std::size_t Size(int key) const noexcept;

			/**
			 * @brief Whether bucket key cannot accept another Push without waiting.
			 * @param key Bucket key.
			 * @return Hopper::Full, or false if the bucket does not exist.
			 */
			bool Full(int key) const noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Consumer
			 * @{
			 */

			/**
			 * @brief Pops one item. No key argument.
			 * @return Next pointer, or empty. Empty plus EoF is end.
			 *
			 * Blocks while there are zero buckets and the Sink is
			 * not closed.
			 */
			std::shared_ptr<StormByte::Multimedia::Pipeline::Item> Pop() noexcept;

			/**
			 * @brief Pops one item with an order-index selector.
			 * @param select Index chooser. Empty: round-robin.
			 * @return Next pointer, or empty.
			 *
			 * Blocks while there are zero buckets and the Sink is
			 * not closed. Ignored when there is one bucket. select
			 * sees [0, bucket-count), not keys. Count includes
			 * teed hoppers on this sink.
			 */
			std::shared_ptr<StormByte::Multimedia::Pipeline::Item> Pop(const Select& select) noexcept;

			/**
			 * @brief Whether the consumer may treat this sink as finished.
			 * @return true if the Sink is closed and every existing
			 *         hopper is Hopper::EoF and empty. Closed with
			 *         zero buckets is true.
			 *
			 * There is no per-key EoF. Does not pop. Looks at main
			 * hoppers and incoming taps.
			 */
			bool EoF() const noexcept;

			/**
			 * @brief Whether the consumer waiter may return.
			 * @return true if a Pop would see an item, or EoF.
			 *
			 * Zero buckets: true only if the Sink is closed.
			 */
			bool Ready() const noexcept;

			/**
			 * @}
			 */

		private:
			/**
			 * @class Tap
			 * @brief One extra hopper filled by Push of @ref key.
			 */
			struct Tap {
				int key = -1;
				Sink* consumer = nullptr;
				std::shared_ptr<Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Item>>> hopper;
			};

			/**
			 * @brief Creates the hopper for key if missing.
			 * @param key Bucket key.
			 * @return Shared hopper. Caller holds m_mutex.
			 */
			std::shared_ptr<Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Item>>> Ensure(int key);

			/**
			 * @brief Rebuilds the pop order from m_buckets and m_tapIn.
			 *
			 * Caller holds m_mutex.
			 */
			void RebuildOrder();

			/**
			 * @brief Snapshot of hoppers in pop order.
			 * @return Shared hoppers. Safe to use without m_mutex.
			 */
			std::vector<std::shared_ptr<Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Item>>>> Order() const;

			/**
			 * @brief Hopper for key, or empty.
			 * @param key Bucket key.
			 * @return Shared hopper. Safe to use without m_mutex.
			 */
			std::shared_ptr<Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Item>>> Bucket(int key) const;

			/**
			 * @brief Whether this producer already Tees @p key to @p consumer.
			 * @param key Track key.
			 * @param consumer Tap destination.
			 * @return true if a Tap with that pair exists.
			 *
			 * Caller holds both mutexes or this Sink mutex.
			 */
			bool HasTap(int key, const Sink& consumer) const noexcept;

			mutable std::mutex m_mutex;				///< Guards the map and taps
			std::condition_variable m_wired;		///< Waits for Bind of a bucket
			std::map<int, std::shared_ptr<Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Item>>>> m_buckets;
			std::vector<std::shared_ptr<Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Item>>>> m_order;
			std::vector<Tap> m_taps;
			std::vector<std::shared_ptr<Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Item>>>> m_tapIn;
			std::atomic<std::size_t> m_rr;
			std::atomic<std::condition_variable*> m_consumer;
			std::atomic<bool> m_closed;
			std::atomic<bool> m_drain;				///< Push without bucket discards
	};
}
