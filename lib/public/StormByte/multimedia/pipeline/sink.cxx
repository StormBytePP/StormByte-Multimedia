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

#include <StormByte/multimedia/pipeline/sink.hxx>

#include <utility>

using namespace StormByte::Multimedia::Pipeline;

Sink::Sink() noexcept
: m_rr(0), m_wake(nullptr) {}

Sink::~Sink() noexcept {
	m_wired.notify_all();
}

void Sink::Push(std::shared_ptr<Item> item) noexcept {
	if (!item)
		return;
	Push(item->Track(), std::move(item));
}

void Sink::Push(int track, std::shared_ptr<Item> item) noexcept {
	if (!item)
		return;
	std::shared_ptr<Multimedia::Buffer::Hopper<std::shared_ptr<Item>>> hopper;
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_wired.wait(lock, [this, track] {
			return m_buckets.find(track) != m_buckets.end();
		});
		hopper = m_buckets[track];
	}
	hopper->Push(std::move(item));
}

void Sink::Eof() noexcept {
	const auto hoppers = Order();
	for (auto& hopper : hoppers)
		hopper->Eof();
}

void Sink::Wake(std::condition_variable& wake) noexcept {
	m_wake.store(&wake, std::memory_order_release);
	const auto hoppers = Order();
	for (auto& hopper : hoppers)
		hopper->Notify(wake);
}

void Sink::Bind(Sink& consumer) {
	std::condition_variable* wake = consumer.m_wake.load(std::memory_order_acquire);
	std::scoped_lock lock(m_mutex, consumer.m_mutex);
	for (auto& [track, hopper] : m_buckets) {
		if (wake != nullptr)
			hopper->Notify(*wake);
		consumer.m_buckets[track] = hopper;
	}
	consumer.RebuildOrder();
	consumer.m_wired.notify_all();
	m_wired.notify_all();
}

void Sink::Bind(int track, Sink& consumer) {
	std::condition_variable* wake = consumer.m_wake.load(std::memory_order_acquire);
	std::scoped_lock lock(m_mutex, consumer.m_mutex);
	auto hopper = Ensure(track);
	if (wake != nullptr)
		hopper->Notify(*wake);
	consumer.m_buckets[track] = hopper;
	consumer.RebuildOrder();
	consumer.m_wired.notify_all();
	m_wired.notify_all();
}

std::shared_ptr<Item> Sink::Pop() noexcept {
	return Pop(Select{});
}

std::shared_ptr<Item> Sink::Pop(const Select& select) noexcept {
	std::vector<std::shared_ptr<Multimedia::Buffer::Hopper<std::shared_ptr<Item>>>> hoppers;
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_wired.wait(lock, [this] {
			return !m_order.empty();
		});
		hoppers = m_order;
	}
	if (hoppers.size() == 1)
		return hoppers.front()->Pop();

	const std::size_t count = hoppers.size();
	std::size_t start = 0;
	if (select)
		start = select(count) % count;
	else
		start = m_rr.fetch_add(1, std::memory_order_relaxed) % count;

	for (std::size_t offset = 0; offset < count; ++offset) {
		std::shared_ptr<Item> item = hoppers[(start + offset) % count]->Pop();
		if (item)
			return item;
	}
	return {};
}

bool Sink::EoF() const noexcept {
	const auto hoppers = Order();
	if (hoppers.empty())
		return false;
	for (const auto& hopper : hoppers) {
		if (!hopper->EoF())
			return false;
		if (!hopper->Empty())
			return false;
	}
	return true;
}

bool Sink::Ready() const noexcept {
	const auto hoppers = Order();
	if (hoppers.empty())
		return false;
	bool drained = true;
	for (const auto& hopper : hoppers) {
		if (!hopper->Empty())
			return true;
		if (!hopper->EoF())
			drained = false;
	}
	return drained;
}

std::shared_ptr<Multimedia::Buffer::Hopper<std::shared_ptr<Item>>> Sink::Ensure(int track) {
	auto found = m_buckets.find(track);
	if (found != m_buckets.end())
		return found->second;
	auto hopper = std::make_shared<Multimedia::Buffer::Hopper<std::shared_ptr<Item>>>();
	std::condition_variable* wake = m_wake.load(std::memory_order_acquire);
	if (wake != nullptr)
		hopper->Notify(*wake);
	m_buckets.emplace(track, hopper);
	RebuildOrder();
	return hopper;
}

void Sink::RebuildOrder() {
	m_order.clear();
	m_order.reserve(m_buckets.size());
	for (auto& [track, hopper] : m_buckets)
		m_order.push_back(hopper);
}

std::vector<std::shared_ptr<Multimedia::Buffer::Hopper<std::shared_ptr<Item>>>> Sink::Order() const {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_order;
}
