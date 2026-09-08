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

#include <StormByte/multimedia/pipeline/engine/transcode/engine.hxx>

namespace StormByte::Multimedia::Pipeline::Engine::Transcode {
	namespace {
		std::size_t AtLeastOne(std::size_t value) noexcept {
			return value == 0 ? 1 : value;
		}
	}

	template<typename Item>
	BoundQueue<Item>::BoundQueue(std::size_t ceiling) noexcept
	: m_ceiling(AtLeastOne(ceiling)) {}

	template<typename Item>
	void BoundQueue<Item>::Push(Item item, const std::atomic_bool& cancel) noexcept {
		std::unique_lock lock(m_mutex);
		m_cv.wait(lock, [&]() {
			return cancel.load(std::memory_order_acquire)
				|| m_closed
				|| m_queue.size() < m_ceiling;
		});
		if (cancel.load(std::memory_order_acquire) || m_closed)
			return;
		m_queue.push_back(std::move(item));
		lock.unlock();
		m_cv.notify_one();
	}

	template<typename Item>
	bool BoundQueue<Item>::TryPush(Item item) noexcept {
		std::lock_guard lock(m_mutex);
		if (m_closed || m_queue.size() >= m_ceiling)
			return false;
		m_queue.push_back(std::move(item));
		m_cv.notify_one();
		return true;
	}

	template<typename Item>
	std::optional<Item> BoundQueue<Item>::Pop(const std::atomic_bool& cancel) noexcept {
		std::unique_lock lock(m_mutex);
		m_cv.wait(lock, [&]() {
			return cancel.load(std::memory_order_acquire)
				|| m_closed
				|| !m_queue.empty();
		});
		if (cancel.load(std::memory_order_acquire))
			return std::nullopt;
		if (m_queue.empty())
			return std::nullopt;
		Item item = std::move(m_queue.front());
		m_queue.pop_front();
		lock.unlock();
		m_cv.notify_all();
		return item;
	}

	template<typename Item>
	std::optional<Item> BoundQueue<Item>::TryPop() noexcept {
		std::lock_guard lock(m_mutex);
		if (m_queue.empty())
			return std::nullopt;
		Item item = std::move(m_queue.front());
		m_queue.pop_front();
		m_cv.notify_all();
		return item;
	}

	template<typename Item>
	bool BoundQueue<Item>::Empty() const noexcept {
		std::lock_guard lock(m_mutex);
		return m_queue.empty();
	}

	template<typename Item>
	bool BoundQueue<Item>::Full() const noexcept {
		std::lock_guard lock(m_mutex);
		return m_queue.size() >= m_ceiling;
	}

	template<typename Item>
	bool BoundQueue<Item>::Closed() const noexcept {
		std::lock_guard lock(m_mutex);
		return m_closed && m_queue.empty();
	}

	template<typename Item>
	void BoundQueue<Item>::Close() noexcept {
		{
			std::lock_guard lock(m_mutex);
			m_closed = true;
		}
		m_cv.notify_all();
	}

	template<typename Item>
	void BoundQueue<Item>::Wake() noexcept {
		m_cv.notify_all();
	}

	template class BoundQueue<StormByte::Multimedia::Pipeline::Packet>;
	template class BoundQueue<StormByte::Multimedia::Pipeline::Frame>;

	Engine::Engine() noexcept = default;

	Engine::~Engine() noexcept {
		RequestCancel();
		Join();
	}

	void Engine::NotifyIntake() noexcept {
		intakeCv.notify_all();
	}

	void Engine::WaitForIntake() noexcept {
		std::unique_lock wait(intakeMutex);
		intakeCv.wait(wait, [&]() {
			return cancel.load(std::memory_order_acquire)
				|| (muxQueue && !muxQueue->Empty())
				|| (copyQueue && !copyQueue->Empty())
				|| ((muxQueue && muxQueue->Closed())
					&& (copyQueue && copyQueue->Closed()));
		});
	}

	void Engine::RequestCancel() noexcept {
		cancel.store(true, std::memory_order_release);
		paused.store(false, std::memory_order_release);
		pauseCv.notify_all();
		NotifyIntake();
		if (muxQueue)
			muxQueue->Wake();
		if (copyQueue)
			copyQueue->Wake();
		for (auto& queue : encodeQueues) {
			if (queue)
				queue->Wake();
		}
		for (auto& queue : frameQueues) {
			if (queue)
				queue->Wake();
		}
	}

	void Engine::Join() noexcept {
		if (worker.joinable())
			worker.join();
	}

	void Engine::WaitIfPaused() noexcept {
		std::unique_lock wait(pauseMutex);
		pauseCv.wait(wait, [&]() {
			return cancel.load(std::memory_order_acquire)
				|| !paused.load(std::memory_order_acquire);
		});
	}
}
