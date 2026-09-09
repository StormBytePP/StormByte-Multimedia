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
#include <memory>
#include <mutex>
#include <queue>
#include <utility>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Item;		///< Tube facade.
	class Frame;	///< Decoded unit.
	class Packet;	///< Compressed unit.

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
	 * @brief N-N queue of @ref SmartPointer items inside one Sink bucket.
	 *
	 * Lock-free V1 contract: any number of producers and consumers,
	 * including zero and one. This compilation uses a mutex; the
	 * lock-free body moves to Buffer.
	 *
	 * Instantiated only for @c shared_ptr of @ref Item, @ref Frame
	 * and @ref Packet. User code does not instantiate this template.
	 *
	 * The hopper does not own a condition variable. Bind stores the
	 * consumer step's CV. @ref Push and @ref Eof notify only when that
	 * pointer is set. Hold is a thread-local queue on FFmpeg, not this
	 * type.
	 *
	 * @tparam T @ref SmartPointer stored in the bucket.
	 *
	 * @todo Buffer.
	 *
	 * @ingroup multimedia_pipeline
	 */
	template<SmartPointer T>
	class STORMBYTE_MULTIMEDIA_PUBLIC Hopper {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Empty hopper with no consumer CV yet.
			 */
			Hopper() noexcept
			: m_eof(false), m_wake(nullptr) {}

			/**
			 * @brief Hopper already pointed at a consumer CV.
			 * @param wake Consumer step condition variable.
			 */
			explicit Hopper(std::condition_variable& wake) noexcept
			: m_eof(false), m_wake(&wake) {}

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
			 * @brief Destructor.
			 */
			~Hopper() noexcept = default;

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
			 * @name Producer
			 * @{
			 */

			/**
			 * @brief Enqueues one item and wakes a waiter if Bind already ran.
			 * @param item Pointer to push. Empty pointers are discarded.
			 */
			void Push(T item) noexcept {
				if (!item)
					return;
				{
					std::lock_guard<std::mutex> lock(m_mutex);
					m_items.push(std::move(item));
				}
				Notify();
			}

			/**
			 * @brief Marks end of input and wakes a waiter.
			 */
			void Eof() noexcept {
				m_eof.store(true, std::memory_order_release);
				Notify();
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
			 */
			T Pop() noexcept {
				std::lock_guard<std::mutex> lock(m_mutex);
				if (m_items.empty())
					return T{};
				T item = std::move(m_items.front());
				m_items.pop();
				return item;
			}

			/**
			 * @brief Whether the producer called @ref Eof.
			 * @return true after @ref Eof.
			 */
			bool EoF() const noexcept {
				return m_eof.load(std::memory_order_acquire);
			}

			/**
			 * @brief Whether the queue has no pending items.
			 * @return true if empty. Does not pop.
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
			 * @brief Points this hopper at the consumer step CV.
			 * @param wake Consumer @c Wake() CV. Not owned.
			 */
			void Wake(std::condition_variable& wake) noexcept {
				m_wake.store(&wake, std::memory_order_release);
			}

			/**
			 * @}
			 */

		private:
			/**
			 * @brief Notifies the consumer CV when Bind has set one.
			 */
			void Notify() noexcept {
				std::condition_variable* wake = m_wake.load(std::memory_order_acquire);
				if (wake == nullptr)
					return;
				wake->notify_one();
			}

			mutable std::mutex m_mutex;							///< Guards @ref m_items
			std::queue<T> m_items;								///< Pending pointers
			std::atomic<bool> m_eof;							///< Producer called @ref Eof
			std::atomic<std::condition_variable*> m_wake;		///< Consumer CV; Bind sets it
	};

	extern template class STORMBYTE_MULTIMEDIA_PRIVATE Hopper<std::shared_ptr<Item>>;
	extern template class STORMBYTE_MULTIMEDIA_PRIVATE Hopper<std::shared_ptr<Frame>>;
	extern template class STORMBYTE_MULTIMEDIA_PRIVATE Hopper<std::shared_ptr<Packet>>;
}
