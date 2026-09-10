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

#include <StormByte/multimedia/visibility.h>

#include <atomic>
#include <concepts>
#include <condition_variable>
#include <cstddef>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>

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
	class Item;		///< Tube facade. Defined in pipeline/item.hxx
	class Frame;	///< Decoded unit. Defined in pipeline/frame.hxx
	class Packet;	///< Compressed unit. Defined in pipeline/packet.hxx
}

/**
 * @namespace StormByte::Multimedia::Buffer
 * @brief Private tube queues.
 *
 * Hopper is one SPSC bucket. Sink maps origin tracks to hoppers.
 * Not a media source; that Buffer is a later type.
 *
 * @ingroup multimedia_buffer
 */
namespace StormByte::Multimedia::Buffer {
	/**
	 * @concept SmartPointer
	 * @brief Pointer that is empty-testable and dereferenceable.
	 *
	 * Covers @c std::shared_ptr and @c std::unique_ptr.
	 *
	 * @todo Move to StormByte (base).
	 */
	template<typename T>
	concept SmartPointer = requires(T p) {
		{ static_cast<bool>(p) } -> std::convertible_to<bool>;
		*p;
	};

	/**
	 * @class Hopper
	 * @brief One SPSC queue of @ref SmartPointer items. One Sink bucket.
	 *
	 * Visibility @c STORMBYTE_MULTIMEDIA_PRIVATE. User code does not
	 * instantiate or include this header.
	 *
	 * One producer thread, one consumer thread. Fan-out is Sink
	 * (N buckets, one per origin track), not this type.
	 *
	 * The queue is a mutex plus @c std::queue. A lock-free body is
	 * a later Buffer change; these method names stay.
	 *
	 * Instantiated only for @c shared_ptr of
	 * @ref StormByte::Multimedia::Pipeline::Item,
	 * @ref StormByte::Multimedia::Pipeline::Frame and
	 * @ref StormByte::Multimedia::Pipeline::Packet.
	 *
	 * Construction does not take a condition variable. @ref Notify
	 * stores the consumer Step CV at Bind (the consumer may not
	 * exist yet). @ref Push and @ref Eof signal that CV once it is
	 * set. A second CV (@c m_space) lives on this object: @ref Push
	 * waits there when @ref Capacity is non-zero and the bucket is
	 * full. @ref Pop wakes one producer. @ref Eof, the destructor
	 * and @ref Capacity(std::size_t) wake every producer.
	 *
	 * Filter Hold is a thread-local queue on Filter::FFmpeg, not
	 * this type. There is no Clear / Drop: discarding in-flight
	 * items would punch a hole in the timeline.
	 *
	 * Copy and move are deleted. @c std::mutex and
	 * @c std::condition_variable are not movable; a waiter on
	 * @c m_space plus a move is use-after-free. Sink shares hoppers
	 * with @c shared_ptr.
	 *
	 * @tparam T @ref SmartPointer stored in the bucket.
	 *
	 * @ingroup multimedia_buffer
	 */
	template<SmartPointer T>
	class STORMBYTE_MULTIMEDIA_PRIVATE Hopper {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Empty unbounded hopper. No consumer CV until @ref Notify.
			 */
			Hopper() noexcept
			: m_eof(false), m_wake(nullptr), m_cap(0) {}

			/**
			 * @brief Empty hopper with a ceiling.
			 * @param capacity Max items. @c 0 is unbounded.
			 */
			explicit Hopper(std::size_t capacity) noexcept
			: m_eof(false), m_wake(nullptr), m_cap(capacity) {}

			/**
			 * @brief Copy constructor.
			 * @param other Source hopper.
			 */
			Hopper(const Hopper& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Hopper to take.
			 */
			Hopper(Hopper&& other) noexcept = delete;

			/**
			 * @brief Destructor. Wakes any Push blocked on a full bucket.
			 */
			~Hopper() noexcept {
				m_eof.store(true, std::memory_order_release);
				m_space.notify_all();
			}

			/**
			 * @brief Copy assignment.
			 * @param other Source hopper.
			 * @return *this.
			 */
			Hopper& operator=(const Hopper& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Hopper to take.
			 * @return *this.
			 */
			Hopper& operator=(Hopper&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @name Capacity
			 * @{
			 */

			/**
			 * @brief Current ceiling.
			 * @return Max items, or @c 0 if unbounded.
			 */
			std::size_t Capacity() const noexcept {
				return m_cap.load(std::memory_order_acquire);
			}

			/**
			 * @brief Sets the ceiling.
			 * @param capacity Max items. @c 0 is unbounded.
			 *
			 * Safe at any time. Lowering does not drop queued items;
			 * the next @ref Push waits until @ref Size is below the
			 * new ceiling. Call from Bind before the first Push when
			 * possible.
			 */
			void Capacity(std::size_t capacity) noexcept {
				m_cap.store(capacity, std::memory_order_release);
				m_space.notify_all();
			}

			/**
			 * @brief Items waiting in the bucket.
			 * @return Count. Does not pop.
			 */
			std::size_t Size() const noexcept {
				std::lock_guard<std::mutex> lock(m_mutex);
				return m_items.size();
			}

			/**
			 * @brief Whether a bounded bucket cannot accept another Push without waiting.
			 * @return true if @ref Capacity is not 0 and @ref Size >= Capacity.
			 */
			bool Full() const noexcept {
				const std::size_t cap = m_cap.load(std::memory_order_acquire);
				if (cap == 0)
					return false;
				return Size() >= cap;
			}

			/**
			 * @}
			 */

			/**
			 * @name Producer
			 * @{
			 */

			/**
			 * @brief Enqueues one item and signals the consumer if @ref Notify ran.
			 * @param item Pointer to push. Empty pointers are discarded.
			 *
			 * If @ref Capacity is not 0 and the bucket is full, waits
			 * on @c m_space until @ref Pop, @ref Eof or destruction.
			 * After @ref Eof the item is not queued.
			 */
			void Push(T item) noexcept {
				if (!item)
					return;
				{
					std::unique_lock<std::mutex> lock(m_mutex);
					m_space.wait(lock, [this]() {
						const std::size_t cap = m_cap.load(std::memory_order_acquire);
						return cap == 0
							|| m_items.size() < cap
							|| m_eof.load(std::memory_order_acquire);
					});
					if (m_eof.load(std::memory_order_acquire))
						return;
					m_items.push(std::move(item));
				}
				SignalConsumer();
			}

			/**
			 * @brief Marks end of input and wakes consumer and producer waiters.
			 *
			 * Does not discard items already queued. Drain with @ref Pop
			 * until @ref Empty, then treat @ref EoF as tube end.
			 */
			void Eof() noexcept {
				m_eof.store(true, std::memory_order_release);
				SignalConsumer();
				m_space.notify_all();
			}

			/**
			 * @}
			 */

			/**
			 * @name Consumer
			 * @{
			 */

			/**
			 * @brief Pops one item, or empty if the bucket is dry.
			 * @return Next pointer, or empty. Empty plus @ref EoF is tube EoF.
			 *
			 * Does not wait. If this returns empty and @ref EoF is
			 * false, the Step waits on its own CV until the next
			 * @ref Push or @ref Eof. Wakes one producer blocked in
			 * @ref Push.
			 */
			T Pop() noexcept {
				T item{};
				{
					std::lock_guard<std::mutex> lock(m_mutex);
					if (m_items.empty())
						return T{};
					item = std::move(m_items.front());
					m_items.pop();
				}
				m_space.notify_one();
				return item;
			}

			/**
			 * @brief Whether the producer called @ref Eof.
			 * @return true after @ref Eof. The queue may still hold items.
			 */
			bool EoF() const noexcept {
				return m_eof.load(std::memory_order_acquire);
			}

			/**
			 * @brief Whether the queue has no pending items.
			 * @return true if empty. Does not pop. Independent of @ref EoF.
			 */
			bool Empty() const noexcept {
				std::lock_guard<std::mutex> lock(m_mutex);
				return m_items.empty();
			}

			/**
			 * @}
			 */

			/**
			 * @name Bind
			 * @{
			 */

			/**
			 * @brief Points this hopper at the consumer Step CV.
			 * @param wake Consumer Step::Wake() CV. Not owned.
			 *
			 * Call from Bind once the consumer exists. Later @ref Push
			 * and @ref Eof signal this CV. Replacing the pointer is
			 * allowed; the previous CV is not signalled.
			 */
			void Notify(std::condition_variable& wake) noexcept {
				m_wake.store(&wake, std::memory_order_release);
			}

			/**
			 * @}
			 */

		private:
			/**
			 * @brief Signals the consumer CV when @ref Notify has set one.
			 */
			void SignalConsumer() noexcept {
				std::condition_variable* wake = m_wake.load(std::memory_order_acquire);
				if (wake == nullptr)
					return;
				wake->notify_one();
			}

			mutable std::mutex m_mutex;						///< Guards @ref m_items
			std::condition_variable m_space;				///< Producer waits here when full
			std::queue<T> m_items;							///< Pending pointers
			std::atomic<bool> m_eof;						///< Producer called @ref Eof
			std::atomic<std::condition_variable*> m_wake;	///< Consumer CV; @ref Notify sets it
			std::atomic<std::size_t> m_cap;					///< Max items; 0 = unbounded
	};

	extern template class STORMBYTE_MULTIMEDIA_PRIVATE
		Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Item>>;
	extern template class STORMBYTE_MULTIMEDIA_PRIVATE
		Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Frame>>;
	extern template class STORMBYTE_MULTIMEDIA_PRIVATE
		Hopper<std::shared_ptr<StormByte::Multimedia::Pipeline::Packet>>;
}
