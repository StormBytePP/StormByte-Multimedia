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

#include <StormByte/logger/manipulators.hxx>
#include <StormByte/multimedia/backend/pipeline/detail/pumper/through.hxx>
#include <StormByte/multimedia/backend/pipeline/detail/worker/filter.hxx>
#include <StormByte/multimedia/backend/pipeline/frame.hxx>
#include <StormByte/multimedia/backend/pipeline/host.hxx>
#include <StormByte/multimedia/backend/pipeline/packet.hxx>
#include <StormByte/multimedia/backend/pipeline/pipe.hxx>

#include <StormByte/multimedia/name_thread.hxx>
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/type.hxx>

#include <format>
#include <limits>
#include <utility>

using StormByte::Multimedia::Pipeline::Filter::Analytics;
using StormByte::Multimedia::Pipeline::Filter::FFmpeg;
using StormByte::Multimedia::Pipeline::Filter::Packet;
using StormByte::Multimedia::Pipeline::Filter::Process;
using StormByte::Multimedia::Pipeline::Item;
using StormByte::Multimedia::Pipeline::Kind;
using StormByte::Multimedia::Pipeline::Kinds;
using StormByte::Multimedia::Pipeline::State;
using StormByte::Multimedia::ToString;
using StormByte::Logger::Level;

namespace {
	int TrackOf(const StormByte::Multimedia::Pipeline::Item& item) noexcept {
		if (item.Kind() == Kind::Frame)
			return static_cast<const StormByte::Multimedia::Pipeline::Frame&>(item).Track();
		return static_cast<const StormByte::Multimedia::Pipeline::Packet&>(item).Track();
	}

	std::uint64_t PartOf(const StormByte::Multimedia::Pipeline::Item& item) noexcept {
		if (item.Kind() == Kind::Frame)
			return static_cast<const StormByte::Multimedia::Pipeline::Frame&>(item).Part();
		return static_cast<const StormByte::Multimedia::Pipeline::Packet&>(item).Part();
	}

	std::optional<std::uint64_t> SerialOf(const StormByte::Multimedia::Pipeline::Item& item) noexcept {
		if (item.Kind() == Kind::Frame)
			return static_cast<const StormByte::Multimedia::Pipeline::Frame&>(item).Serial();
		return static_cast<const StormByte::Multimedia::Pipeline::Packet&>(item).Serial();
	}

	bool IsAnalytics(const FFmpeg& node) noexcept {
		return dynamic_cast<const Analytics*>(&node) != nullptr;
	}
}

class FFmpeg::Surface final: public StormByte::Multimedia::Backend::Pipeline::Host {
	public:
		explicit Surface(FFmpeg& owner) noexcept
		:	m_owner(owner) {}

		void Emit(Item::PointerType item) noexcept override {
			m_owner.Emit(std::move(item));
		}

		void Wait() noexcept override {
			m_owner.Wait();
		}

		bool Stopping() const noexcept override {
			return m_owner.Stopping();
		}

		void Fail(std::string reason) noexcept override {
			m_owner.Fail(std::move(reason));
		}

		void Log(StormByte::Logger::Level level, std::string_view message) noexcept override {
			m_owner.Log(level, message);
		}

		void Ended() noexcept override {
			m_owner.m_exhausted = true;
		}

		bool Exhausted() const noexcept override {
			return m_owner.m_exhausted;
		}

		Item::PointerType Pull() noexcept override {
			Item::PointerType item;
			m_owner.pipe() >> item;
			return item;
		}

		bool InputEof() const noexcept override {
			return m_owner.pipe().InputEof();
		}

		void CloseOutput() noexcept override {
			m_owner.pipe().Close();
		}

		void BecameReady() noexcept override {
			m_owner.Log(Level::Debug, "ready");
			m_owner.m_wake.notify_all();
		}

		void RecordWork(std::int64_t microseconds) noexcept override {
			m_owner.RecordWork(microseconds);
		}

		void DumpWork() noexcept override {
			m_owner.DumpWork();
		}

	private:
		FFmpeg& m_owner;
};

FFmpeg::FFmpeg(std::shared_ptr<StormByte::Logger::Log> log,
	std::string name, Kinds receives, Kinds produces) noexcept
:	m_log(std::move(log)),
	m_name(std::move(name)),
	m_receives(receives),
	m_produces(produces),
	m_wake(),
	m_pipe(std::make_unique<StormByte::Multimedia::Backend::Pipeline::Pipe>(m_wake)),
	m_surface(std::make_unique<Surface>(*this)),
	m_exhausted(false),
	m_workN(0),
	m_workMin(std::numeric_limits<std::int64_t>::max()),
	m_workMax(0),
	m_lastWork(0),
	m_hold(0),
	m_heldFor(0) {
	m_pumper = std::make_unique<StormByte::Multimedia::Backend::Pipeline::Detail::Pumper::Through>(Face());
	m_pumper->Bind(std::make_unique<StormByte::Multimedia::Backend::Pipeline::Detail::Worker::Filter>(*this));
	if (m_log) {
		*m_log << StormByte::Logger::component("STMM")
			<< StormByte::Logger::group(m_name)
			<< Level::Notice << (m_name + " created") << std::endl;
	}
}

FFmpeg::~FFmpeg() noexcept {
	Halt();
}

std::string FFmpeg::Name() const noexcept {
	return std::string(ToString(Media())) + "/" + m_name;
}

void FFmpeg::Process(const Pipeline::Frame&) noexcept {}

void FFmpeg::Process(const Pipeline::Packet&) noexcept {}

class StormByte::Multimedia::Pipeline::Filter::Report FFmpeg::Report() const noexcept {
	return {};
}

State FFmpeg::Status() const noexcept {
	if (m_pumper)
		return m_pumper->Status();
	return m_error ? State::Failed : State::Created;
}

bool FFmpeg::Failed() const noexcept {
	return Status() == State::Failed;
}

const std::optional<std::string>& FFmpeg::Error() const noexcept {
	return m_error;
}

std::size_t FFmpeg::InputCeiling() const noexcept {
	return 0;
}

void FFmpeg::Log(StormByte::Logger::Level level, std::string_view message) noexcept {
	if (!m_log)
		return;
	*m_log << StormByte::Logger::component("STMM")
		<< StormByte::Logger::group(Name())
		<< level << std::string(message) << std::endl;
}

void FFmpeg::Fail(std::string reason) noexcept {
	m_hold = 0;
	m_heldFor = 0;
	m_queue.clear();
	m_error = std::move(reason);
	Log(Level::Error, *m_error);
	if (m_pumper)
		m_pumper->Fail(*m_error);
	CloseHoppers();
	m_wake.notify_all();
}

void FFmpeg::Hold(std::uint8_t n) noexcept {
	if (Held()) {
		Fail("Hold while already Held");
		return;
	}

	if (!m_current) {
		Fail("Hold without a unit");
		return;
	}

	m_hold = n == 0 ? std::numeric_limits<std::uint8_t>::max() : n;
	m_heldFor = 0;
	Log(Level::Debug, std::format("{} hold n={}", Name(), static_cast<unsigned>(m_hold)));
	Park();
}

void FFmpeg::Release() noexcept {
	if (!Held())
		return;
	Log(Level::Debug, std::format("{} release held={}", Name(), static_cast<unsigned>(m_heldFor)));
	m_hold = 0;
	m_heldFor = 0;
	auto parked = std::move(m_queue);
	for (auto& item : parked) {
		m_current = item;
		if (IsAnalytics(*this)) {
			Work(m_current);
			continue;
		}

		if (m_current->Kind() == Pipeline::Kind::Frame)
			Process(static_cast<const Pipeline::Frame&>(*m_current));
		else
			Process(static_cast<const Pipeline::Packet&>(*m_current));
		if (Failed())
			return;
		if (m_current)
			Emit(std::move(m_current));
	}

	m_current.reset();
}

bool FFmpeg::Held() const noexcept {
	return m_hold > 0;
}

std::uint8_t FFmpeg::HeldFor() const noexcept {
	return m_heldFor;
}

void FFmpeg::Eof() noexcept {}

const StormByte::Multimedia::FFmpeg::AVFrame& FFmpeg::AVFrame() const noexcept {
	static StormByte::Multimedia::FFmpeg::AVFrame empty;
	static const bool primed = []() noexcept {
		empty.Reset(nullptr);
		return true;
	}();
	(void)primed;
	auto frame = std::dynamic_pointer_cast<const Pipeline::Frame>(m_current);
	if (!frame || !frame->m_backend)
		return empty;
	return frame->m_backend->Handle();
}

const StormByte::Multimedia::FFmpeg::AVPacket& FFmpeg::AVPacket() const noexcept {
	static StormByte::Multimedia::FFmpeg::AVPacket empty;
	static const bool primed = []() noexcept {
		empty.Reset(nullptr);
		return true;
	}();
	(void)primed;
	auto packet = std::dynamic_pointer_cast<const Pipeline::Packet>(m_current);
	if (!packet || !packet->m_backend)
		return empty;
	return packet->m_backend->Handle();
}

void FFmpeg::Save(StormByte::Multimedia::FFmpeg::AVFrame&& incoming) noexcept {
	auto frame = std::dynamic_pointer_cast<Pipeline::Frame>(m_current);
	if (!frame)
		return;
	if (!frame->m_backend)
		frame->m_backend = std::make_unique<StormByte::Multimedia::Backend::Pipeline::Frame>();
	frame->m_backend->Put(*frame, incoming.Detach());
	if (!frame->m_backend->Warning().empty())
		Log(Level::Warning, std::format("{}: {}", Name(), frame->m_backend->Warning()));
	Log(Level::LowLevel, std::format("{} save frame t={} {}:{}",
		Name(), frame->Track(), frame->Serial().value_or(0), frame->Part()));
}

void FFmpeg::Save(StormByte::Multimedia::FFmpeg::AVPacket&& incoming) noexcept {
	auto packet = std::dynamic_pointer_cast<Pipeline::Packet>(m_current);
	if (!packet)
		return;
	if (!packet->m_backend)
		packet->m_backend = std::make_unique<StormByte::Multimedia::Backend::Pipeline::Packet>();
	packet->m_backend->Handle().Reset(incoming.Detach());
	packet->m_backend->BindProperties(*packet);
	Log(Level::LowLevel, std::format("{} save packet t={} {}:{}",
		Name(), packet->Track(), packet->Serial().value_or(0), packet->Part()));
}

void FFmpeg::Open() noexcept {
	NameThread("STMM:FFmpeg:" + m_name);
	Log(Level::Notice, Name() + " setup");
	Clean();
	Setup();
}

void FFmpeg::LastChance(const Pipeline::Frame&) noexcept {}

void FFmpeg::LastChance(const Pipeline::Packet&) noexcept {}

void FFmpeg::Park() noexcept {
	if (!m_current)
		return;
	if (!m_queue.empty() && m_queue.back() == m_current)
		return;
	if (m_heldFor >= m_hold) {
		Fail("Hold exceeded");
		return;
	}

	m_queue.push_back(m_current);
	++m_heldFor;
}

void FFmpeg::CallLastChance() noexcept {
	if (!m_current)
		return;
	Log(Level::Debug, Name() + " last-chance");
	if (m_current->Kind() == Pipeline::Kind::Frame)
		LastChance(static_cast<const Pipeline::Frame&>(*m_current));
	else if (!IsAnalytics(*this))
		LastChance(static_cast<const Pipeline::Packet&>(*m_current));
}

void FFmpeg::Work(Pipeline::Item::PointerType item) noexcept {
	m_current = std::move(item);
	Log(Level::LowLevel, std::format("{} in t={} {}:{}",
		Name(), TrackOf(*m_current), SerialOf(*m_current).value_or(0), PartOf(*m_current)));

	if (IsAnalytics(*this)) {
		if (m_current->Kind() == Pipeline::Kind::Frame)
			Process(static_cast<const Pipeline::Frame&>(*m_current));
		if (Failed())
			return;
		if (m_current)
			Emit(std::move(m_current));
		return;
	}

	if (m_current->Kind() == Pipeline::Kind::Frame)
		Process(static_cast<const Pipeline::Frame&>(*m_current));
	else
		Process(static_cast<const Pipeline::Packet&>(*m_current));
	if (Failed())
		return;
	if (Held()) {
		if (m_heldFor >= m_hold) {
			CallLastChance();
			if (Held()) {
				Fail("Hold exceeded");
				return;
			}
		}

		else {
			Park();
			return;
		}
	}

	if (m_current)
		Emit(std::move(m_current));
}

void FFmpeg::Finish() noexcept {
	if (Held()) {
		if (!m_current && !m_queue.empty())
			m_current = m_queue.back();
		CallLastChance();
		if (Held())
			Fail("Hold + EoF without Release");
	}

	if (!Failed())
		Eof();
}

void FFmpeg::Emit(Pipeline::Item::PointerType item) noexcept {
	item >> *m_pipe;
}

void FFmpeg::Wait() noexcept {
	Log(Level::LowLevel, "wait");
	std::unique_lock lock(m_wait);
	m_wake.wait(lock, [this] {
		return Stopping() || m_pipe->Ready();
	});
	Log(Level::LowLevel, "wake");
}

void FFmpeg::Launch() noexcept {
	if (!m_pumper || Stopping())
		return;
	Log(Level::LowLevel, "launch");
	m_pipe->Listen();
	m_pumper->Launch();
}

void FFmpeg::Halt() noexcept {
	Stop();
	if (m_pumper)
		m_pumper->Halt();
}

void FFmpeg::Stop() noexcept {
	const State state = Status();
	const bool signaled = state == State::Created || state == State::Ready;
	if (m_pumper)
		m_pumper->Stop();
	if (signaled)
		Log(Level::LowLevel, "stop");
	CloseHoppers();
	m_wake.notify_all();
}

bool FFmpeg::Stopping() const noexcept {
	const State state = Status();
	return state == State::Stopping || state == State::Stopped || state == State::Failed;
}

StormByte::Multimedia::Backend::Pipeline::Host& FFmpeg::Face() noexcept {
	return *m_surface;
}

std::condition_variable& FFmpeg::Wake() noexcept {
	return m_wake;
}

StormByte::Multimedia::Backend::Pipeline::Pipe& FFmpeg::pipe() noexcept {
	return *m_pipe;
}

const StormByte::Multimedia::Backend::Pipeline::Pipe& FFmpeg::pipe() const noexcept {
	return *m_pipe;
}

void FFmpeg::CloseHoppers() noexcept {
	m_pipe->Close();
}

void FFmpeg::RecordWork(std::int64_t microseconds) noexcept {
	if (microseconds < 0)
		microseconds = 0;
	++m_workN;
	m_lastWork = microseconds;
	if (microseconds < m_workMin)
		m_workMin = microseconds;
	if (microseconds > m_workMax)
		m_workMax = microseconds;
}

void FFmpeg::DumpWork() noexcept {
	if (m_workN == 0)
		return;
	Log(Level::Debug, std::format("work n={} min={}us max={}us",
		m_workN, m_workMin, m_workMax));
}

Process::Process(std::shared_ptr<StormByte::Logger::Log> log, std::string name) noexcept
: FFmpeg(std::move(log), std::move(name), Kinds{Kind::Frame}, Kinds{Kind::Frame}) {}

Process::Process(std::shared_ptr<StormByte::Logger::Log> log, std::string name,
	Kinds receives, Kinds produces) noexcept
: FFmpeg(std::move(log), std::move(name), receives, produces) {}

Packet::Packet(std::shared_ptr<StormByte::Logger::Log> log, std::string name) noexcept
: FFmpeg(std::move(log), std::move(name), Kinds{Kind::Packet}, Kinds{Kind::Packet}) {}

Packet::Packet(std::shared_ptr<StormByte::Logger::Log> log, std::string name,
	Kinds receives, Kinds produces) noexcept
: FFmpeg(std::move(log), std::move(name), receives, produces) {}

Analytics::Analytics(std::shared_ptr<StormByte::Logger::Log> log, std::string name) noexcept
: FFmpeg(std::move(log), std::move(name),
	Kinds{Kind::Frame},
	Kinds{Kind::Frame}) {}

Analytics::Analytics(std::shared_ptr<StormByte::Logger::Log> log, std::string name,
	Kinds receives, Kinds produces) noexcept
: FFmpeg(std::move(log), std::move(name), receives, produces) {}
