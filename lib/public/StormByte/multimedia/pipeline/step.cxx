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

#include <StormByte/multimedia/buffer/sink.hxx>
#include <StormByte/multimedia/pipeline/step.hxx>

using namespace StormByte::Multimedia::Pipeline;

namespace {
	bool Terminal(State state) noexcept {
		return state == State::Failed || state == State::Stopped;
	}
}

Step::Step(Kinds receives, Kinds produces) noexcept
: m_in(std::make_unique<Buffer::Sink>()),
	m_out(std::make_unique<Buffer::Sink>()),
	m_receives(receives),
	m_produces(produces),
	m_state(State::Created) {}

Step::~Step() noexcept {
	Halt();
}

State Step::Status() const noexcept {
	return m_state.load(std::memory_order_acquire);
}

void Step::Fail(std::string reason) noexcept {
	m_error = std::move(reason);
	State current = m_state.load(std::memory_order_acquire);
	while (!Terminal(current)) {
		if (m_state.compare_exchange_weak(current, State::Failed,
				std::memory_order_acq_rel, std::memory_order_acquire))
			break;
	}
	m_in->Eof();
	m_out->Eof();
	m_wake.notify_all();
}

bool Step::Failed() const noexcept {
	return Status() == State::Failed;
}

bool Step::Ready() const noexcept {
	return Status() == State::Ready;
}

void Step::Stop() noexcept {
	State current = m_state.load(std::memory_order_acquire);
	while (current == State::Created || current == State::Ready) {
		if (m_state.compare_exchange_weak(current, State::Stopping,
				std::memory_order_acq_rel, std::memory_order_acquire))
			break;
	}
	m_in->Eof();
	m_out->Eof();
	m_wake.notify_all();
}

std::size_t Step::InputCeiling() const noexcept {
	return 0;
}

bool Step::Stopping() const noexcept {
	const State state = Status();
	return state == State::Stopping || state == State::Stopped || state == State::Failed;
}

const std::optional<std::string>& Step::Error() const noexcept {
	return m_error;
}

std::condition_variable& Step::Wake() noexcept {
	return m_wake;
}

void Step::Wait() noexcept {
	std::unique_lock lock(m_wait);
	m_wake.wait(lock, [this] {
		return Stopping() || m_in->Ready();
	});
}

void Step::Open() noexcept {
	State expected = State::Created;
	m_state.compare_exchange_strong(expected, State::Ready,
		std::memory_order_acq_rel, std::memory_order_acquire);
	m_wake.notify_all();
}

void Step::Work(std::shared_ptr<Item>) noexcept {}

void Step::Finish() noexcept {}

void Step::Pump() noexcept {
	for (;;) {
		if (Stopping())
			break;
		std::shared_ptr<Item> item = m_in->Pop();
		if (!item) {
			if (!m_in->EoF() && !Stopping()) {
				Wait();
				continue;
			}
			if (!Stopping())
				Finish();
			break;
		}
		Work(std::move(item));
	}
	m_out->Eof();
}

void Step::Launch() noexcept {
	if (m_worker.joinable())
		return;
	m_in->Notify(m_wake);
	m_worker = std::jthread([this](std::stop_token token) {
		(void)token;
		Open();
		if (Stopping())
			return;
		Pump();
		m_out->Eof();
		State expected = State::Stopping;
		m_state.compare_exchange_strong(expected, State::Stopped,
			std::memory_order_acq_rel, std::memory_order_acquire);
	});
}

void Step::Halt() noexcept {
	Stop();
	if (m_worker.joinable()) {
		m_worker.request_stop();
		m_worker.join();
	}
	State expected = State::Stopping;
	m_state.compare_exchange_strong(expected, State::Stopped,
		std::memory_order_acq_rel, std::memory_order_acquire);
}

Step& StormByte::Multimedia::Pipeline::operator>>(Step& from, Step& to) noexcept {
	if (!to.m_plan)
		to.m_plan = from.m_plan;
	to.m_in->Notify(to.Wake());
	from.m_out->Bind(*to.m_in);
	return to;
}
