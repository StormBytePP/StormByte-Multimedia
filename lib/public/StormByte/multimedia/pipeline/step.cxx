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

#include <chrono>
#include <format>
#include <string>

using namespace StormByte::Multimedia::Pipeline;
using StormByte::Logger::Level;

namespace {
	bool Terminal(State state) noexcept {
		return state == State::Failed || state == State::Stopped;
	}
}

Step::Step(std::shared_ptr<StormByte::Logger::Log> log,
	enum Producer name,
	Kinds receives, Kinds produces) noexcept
: m_log(std::move(log)),
	m_name(name),
	m_in(std::make_unique<Buffer::Sink>()),
	m_out(std::make_unique<Buffer::Sink>()),
	m_receives(receives),
	m_produces(produces),
	m_state(State::Created),
	m_workN(0),
	m_waitN(0),
	m_workMin(std::numeric_limits<std::int64_t>::max()),
	m_workMax(0),
	m_lastWork(0) {
	Log(Level::Notice, "created");
}

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
	bool signaled = false;
	while (current == State::Created || current == State::Ready) {
		if (m_state.compare_exchange_weak(current, State::Stopping,
				std::memory_order_acq_rel, std::memory_order_acquire)) {
			signaled = true;
			break;
		}
	}
	if (signaled)
		Log(Level::LowLevel, "stop");
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
	++m_waitN;
	const bool talk = ((m_waitN - 1) % SparseWindow) < SparseKeep;
	if (talk)
		Log(Level::LowLevel, "wait");
	std::unique_lock lock(m_wait);
	m_wake.wait(lock, [this] {
		return Stopping() || m_in->Ready();
	});
	if (talk)
		Log(Level::LowLevel, "wake");
	if (m_waitN % SparseWindow == SparseKeep)
		Log(Level::LowLevel, std::format(
			"wait: logged {} wakes, next wait log in {}",
			SparseKeep, SparseWindow - SparseKeep));
}

void Step::Open() noexcept {
	State expected = State::Created;
	m_state.compare_exchange_strong(expected, State::Ready,
		std::memory_order_acq_rel, std::memory_order_acquire);
	Log(Level::Debug, "ready");
	m_wake.notify_all();
}

void Step::Work(std::shared_ptr<Item>) noexcept {}

void Step::Finish() noexcept {}

void Step::Look(StormByte::Multimedia::Buffer::Sink&) noexcept {}

std::string Step::Label() const noexcept {
	return std::string(ToString(m_name));
}

void Step::Log(StormByte::Logger::Level level, std::string_view message) noexcept {
	// TODO(Logger 1.0.1): drop this mutex. StormByte-Logger master already
	// always releases the ThreadedLog line lock on endl (hotfix merged,
	// no release cut yet). Until Multimedia depends on that tag, keep the
	// guard: Implementation still stores current Level / enabled in shared
	// fields, so a concurrent LowLevel write can flip WillWrite() mid-line
	// and, on Logger 1.0.0, skip release_line and stall every other thread.
	if (!m_log)
		return;
	static std::mutex line;
	std::lock_guard<std::mutex> guard(line);
	*m_log << level << std::format("STMM {}: {}", Label(), message) << std::endl;
}

bool Step::Sparse(int track) noexcept {
	const std::uint64_t n = ++m_seen[track];
	return ((n - 1) % SparseWindow) < SparseKeep;
}

void Step::MaybeThrottle(int track) noexcept {
	const std::uint64_t n = m_seen[track];
	if (n == 0 || ((n - 1) % SparseWindow) != SparseKeep)
		return;
	Log(Level::LowLevel, std::format("t={}: logged {} units, next unit log in {}",
		track, SparseKeep, SparseWindow - SparseKeep));
}

void Step::RecordWork(std::int64_t microseconds) noexcept {
	if (microseconds < 0)
		microseconds = 0;
	++m_workN;
	m_lastWork = microseconds;
	if (microseconds < m_workMin)
		m_workMin = microseconds;
	if (microseconds > m_workMax)
		m_workMax = microseconds;
}

std::int64_t Step::LastWork() const noexcept {
	return m_lastWork;
}

void Step::DumpWork() noexcept {
	if (m_workN == 0)
		return;
	Log(Level::Debug, std::format("work n={} min={}us max={}us",
		m_workN, m_workMin, m_workMax));
}

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
			if (!Stopping()) {
				Log(Level::LowLevel, "finish");
				Finish();
				DumpWork();
			}
			break;
		}
		const auto started = std::chrono::steady_clock::now();
		Work(std::move(item));
		const auto us = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::steady_clock::now() - started).count();
		RecordWork(us);
	}
	m_out->Eof();
}

void Step::Launch() noexcept {
	if (m_worker.joinable())
		return;
	Log(Level::LowLevel, "launch");
	m_in->Notify(m_wake);
	m_worker = std::jthread([this](std::stop_token token) {
		(void)token;
		Open();
		if (Stopping())
			return;
		Pump();
		m_out->Eof();
		/*
		* Natural hopper Eof leaves the stage in Ready. Stop() is
		* only Halt/dtor. Route::Idle and Transcoder wait Stopped
		* before Reports; without Ready→Stopped that wait never
		* ends (mux closed, VMAF already pooled).
		*/
		State expected = State::Stopping;
		if (!m_state.compare_exchange_strong(expected, State::Stopped,
				std::memory_order_acq_rel, std::memory_order_acquire)) {
			expected = State::Ready;
			m_state.compare_exchange_strong(expected, State::Stopped,
				std::memory_order_acq_rel, std::memory_order_acquire);
		}
		Log(Level::LowLevel, "stopped");
	});
}

void Step::Halt() noexcept {
	Stop();
	if (m_worker.joinable()) {
		m_worker.request_stop();
		m_worker.join();
	}
	State expected = State::Stopping;
	if (!m_state.compare_exchange_strong(expected, State::Stopped,
			std::memory_order_acq_rel, std::memory_order_acquire)) {
		expected = State::Ready;
		m_state.compare_exchange_strong(expected, State::Stopped,
			std::memory_order_acq_rel, std::memory_order_acquire);
	}
}

Step& StormByte::Multimedia::Pipeline::operator>>(Step& from, Step& to) noexcept {
	if (!to.m_plan)
		to.m_plan = from.m_plan;
	to.m_in->Notify(to.Wake());
	from.m_out->Bind(*to.m_in);
	from.Log(Level::Debug, "bound to " + std::string(ToString(to.m_name)));
	return to;
}
