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

#include <StormByte/multimedia/pipeline/transcode.hxx>
#include <StormByte/multimedia/pipeline/engine/transcode/engine.hxx>

#include <StormByte/expected.hxx>
#include <StormByte/logger/typedefs.hxx>
#include <StormByte/multimedia/pipeline/config/audio.hxx>
#include <StormByte/multimedia/pipeline/config/subtitle.hxx>
#include <StormByte/multimedia/pipeline/config/video.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/stream.hxx>

#include <algorithm>
#include <limits>

using StormByte::Logger::Level;
using StormByte::Multimedia::ExpectedTranscode;
using StormByte::Multimedia::File;
using StormByte::Multimedia::TranscodeException;
using namespace StormByte::Multimedia::Pipeline;

namespace {
	constexpr std::size_t InvalidSlot = std::numeric_limits<std::size_t>::max();

	const StormByte::Multimedia::Stream* FindStream(const File& file, int index) noexcept {
		for (const auto& stream : file.Streams()) {
			if (stream.Index() == index)
				return &stream;
		}
		return nullptr;
	}

	std::string KindName(StormByte::Multimedia::Type type) noexcept {
		return ToString(type);
	}

	Config::Video* AsVideo(Config::Base* config) noexcept {
		return dynamic_cast<Config::Video*>(config);
	}

	Config::Audio* AsAudio(Config::Base* config) noexcept {
		return dynamic_cast<Config::Audio*>(config);
	}

	Config::Subtitle* AsSubtitle(Config::Base* config) noexcept {
		return dynamic_cast<Config::Subtitle*>(config);
	}
}

std::string TrackSettled::ToString() const {
	std::string text = "settled in=" + std::to_string(In)
		+ " out=" + std::to_string(Out);
	text += Destination ? " encode" : " remux";
	if (Source)
		text += std::string(" src=") + std::string(Source->Name());
	if (Destination)
		text += std::string(" dst=") + std::string(Destination->Name());
	if (Implementation)
		text += " impl=" + *Implementation;
	return text;
}

Transcode::Track::Track(Transcode& owner, std::size_t slot) noexcept
: m_owner(&owner), m_slot(slot) {}

Transcode::Track& Transcode::Track::Remux() noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	*m_owner->m_logger << Level::Debug << "track "
		<< m_owner->m_engine->Mapped[m_slot].In << " marked remux" << std::endl;
	return *this;
}

Transcode::Track& Transcode::Track::Codec(const StormByte::Multimedia::Codec& codec) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	auto& slot = m_owner->m_engine->Mapped[m_slot];
	if (codec.Type() != slot.Kind) {
		m_owner->Fail("codec '" + std::string(codec.Name()) + "' is "
			+ KindName(codec.Type()) + ", track " + std::to_string(slot.In)
			+ " is " + KindName(slot.Kind));
		return *this;
	}
	if (auto* video = AsVideo(slot.Config.get()))
		video->Codec(codec);
	else if (auto* audio = AsAudio(slot.Config.get()))
		audio->Codec(codec);
	else if (auto* subtitle = AsSubtitle(slot.Config.get()))
		subtitle->Codec(codec);
	*m_owner->m_logger << Level::Debug << "track " << slot.In << " encode to "
		<< std::string(codec.Name()) << std::endl;
	return *this;
}

Transcode::Track& Transcode::Track::Implementation(std::string name) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	auto& slot = m_owner->m_engine->Mapped[m_slot];
	auto impl = slot.Config->Implementation();
	impl.Encoder = std::move(name);
	slot.Config->Implementation(std::move(impl));
	return *this;
}

Transcode::Track& Transcode::Track::CRF(int value) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	if (auto* video = AsVideo(m_owner->m_engine->Mapped[m_slot].Config.get()))
		video->CRF(value);
	return *this;
}

Transcode::Track& Transcode::Track::BitRate(std::int64_t bits_per_second) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	auto* config = m_owner->m_engine->Mapped[m_slot].Config.get();
	if (auto* video = AsVideo(config))
		video->BitRate(bits_per_second);
	else if (auto* audio = AsAudio(config))
		audio->BitRate(bits_per_second);
	return *this;
}

Transcode::Track& Transcode::Track::MaxBitRate(std::int64_t bits_per_second) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	if (auto* audio = AsAudio(m_owner->m_engine->Mapped[m_slot].Config.get()))
		audio->MaxBitRate(bits_per_second);
	return *this;
}

Transcode::Track& Transcode::Track::Preset(std::string name) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	auto* config = m_owner->m_engine->Mapped[m_slot].Config.get();
	if (auto* video = AsVideo(config))
		video->Preset(std::move(name));
	else if (auto* audio = AsAudio(config))
		audio->Preset(std::move(name));
	return *this;
}

Transcode::Track& Transcode::Track::Tune(std::string name) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	if (auto* video = AsVideo(m_owner->m_engine->Mapped[m_slot].Config.get()))
		video->Tune(std::move(name));
	return *this;
}

Transcode::Track& Transcode::Track::FineTune(std::map<std::string, std::string> options) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	if (auto* video = AsVideo(m_owner->m_engine->Mapped[m_slot].Config.get()))
		video->FineTune(std::move(options));
	return *this;
}

Transcode::Track& Transcode::Track::Language(std::string language) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	m_owner->m_engine->Mapped[m_slot].Config->Language(std::move(language));
	return *this;
}

Transcode::Track& Transcode::Track::Title(std::string title) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	m_owner->m_engine->Mapped[m_slot].Config->Title(std::move(title));
	return *this;
}

Transcode::Transcode(std::shared_ptr<StormByte::Logger::Log> logger, File&& file) noexcept
: m_logger(std::move(logger)), m_file(std::make_unique<File>(std::move(file))),
m_engine(std::make_unique<Engine::Transcode::Engine>()) {
	*m_logger << Level::LowLevel << "Transcode::Transcode" << std::endl;
}

Transcode::~Transcode() noexcept {
	if (m_logger)
		*m_logger << Level::LowLevel << "Transcode::~Transcode" << std::endl;
	if (!m_engine)
		return;
	const auto status = m_engine->Status.load(std::memory_order_acquire);
	if (status == Status::Running || status == Status::Paused)
		Cancel();
	m_engine->Join();
}

void Transcode::Fail(std::string reason) noexcept {
	if (!m_engine)
		return;
	std::lock_guard lock(m_engine->Lock);
	if (m_engine->Status.load(std::memory_order_relaxed) == Status::Error)
		return;
	m_engine->Error = std::move(reason);
	m_engine->Status.store(Status::Error, std::memory_order_release);
	m_engine->RequestCancel();
	if (m_logger)
		*m_logger << Level::Error << *m_engine->Error << std::endl;
}

bool Transcode::ValidSlot(std::size_t slot) const noexcept {
	return m_engine && slot < m_engine->Mapped.size();
}

void Transcode::AttachFilter(std::size_t slot, std::shared_ptr<Filter::FFmpeg> filter) noexcept {
	if (!ValidSlot(slot) || !filter)
		return;
	if (dynamic_cast<Filter::Analytics*>(filter.get()) != nullptr) {
		Fail("Analytics attach on Transcode::Filter, not Track::Filter");
		return;
	}
	m_engine->Mapped[slot].Filters.push_back(std::move(filter));
}

void Transcode::AttachAnalytics(std::shared_ptr<Filter::FFmpeg> filter) noexcept {
	if (!m_engine || !filter)
		return;
	if (dynamic_cast<Filter::Analytics*>(filter.get()) == nullptr) {
		Fail("Track Process/Packet filters attach on Track::Filter");
		return;
	}
	m_engine->Analytics.push_back(std::move(filter));
}

ExpectedTranscode Transcode::Open(std::shared_ptr<StormByte::Logger::Log> logger,
	const std::filesystem::path& source, const std::filesystem::path& destination,
	std::optional<std::chrono::nanoseconds> duration) noexcept {
	if (!logger)
		return StormByte::Unexpected<TranscodeException>("logger is required");
	if (destination.empty())
		return StormByte::Unexpected<TranscodeException>("destination path is empty");
	ExpectedFile opened = duration
		? File::Open(source, *duration)
		: File::Open(source);
	if (!opened) {
		const char* text = opened.error() ? opened.error()->what() : "file open failed";
		*logger << Level::Error << text << std::endl;
		return StormByte::Unexpected<TranscodeException>(text);
	}
	File file = std::move(*opened);
	(void)file.Duration();
	*logger << Level::Notice << "opened source " << file.Path().string() << std::endl;
	auto job = std::unique_ptr<Transcode>(new Transcode(std::move(logger), std::move(file)));
	job->m_engine->Path = destination;
	return job;
}

const File& Transcode::Source() const noexcept {
	if (m_plan)
		return m_plan->Source();
	return *m_file;
}

const std::shared_ptr<StormByte::Logger::Log>& Transcode::Logger() const noexcept {
	return m_logger;
}

Transcode::Track Transcode::AddTrack(int in, int out, Type kind) noexcept {
	*m_logger << Level::LowLevel << "Transcode::AddTrack in=" << in << " out=" << out << std::endl;
	if (out < 0) {
		Fail("destination order key is negative");
		return Track(*this, InvalidSlot);
	}
	const Stream* stream = FindStream(Source(), in);
	if (!stream) {
		Fail("source stream " + std::to_string(in) + " does not exist");
		return Track(*this, InvalidSlot);
	}
	if (stream->Type() != kind) {
		Fail("stream " + std::to_string(in) + " is " + KindName(stream->Type())
			+ ", " + KindName(kind) + "() requires " + KindName(kind));
		return Track(*this, InvalidSlot);
	}
	for (const auto& slot : m_engine->Mapped) {
		if (slot.Out == out) {
			Fail("destination order key " + std::to_string(out) + " is already used");
			return Track(*this, InvalidSlot);
		}
	}
	Engine::Transcode::Slot slot;
	slot.In = in;
	slot.Out = out;
	slot.Kind = kind;
	if (kind == Type::Video)
		slot.Config = std::make_unique<Config::Video>();
	else if (kind == Type::Audio)
		slot.Config = std::make_unique<Config::Audio>();
	else
		slot.Config = std::make_unique<Config::Subtitle>();
	m_engine->Mapped.push_back(std::move(slot));
	*m_logger << Level::Debug << "mapped " << KindName(kind) << " " << in << " -> key " << out << std::endl;
	return Track(*this, m_engine->Mapped.size() - 1);
}

Transcode::Track Transcode::Video(int in, int out) noexcept {
	return AddTrack(in, out, Type::Video);
}

Transcode::Track Transcode::Audio(int in, int out) noexcept {
	return AddTrack(in, out, Type::Audio);
}

Transcode::Track Transcode::Subtitle(int in, int out) noexcept {
	return AddTrack(in, out, Type::Subtitle);
}

Transcode& Transcode::Ignore(int in) noexcept {
	*m_logger << Level::LowLevel << "Transcode::Ignore " << in << std::endl;
	if (!FindStream(Source(), in)) {
		Fail("source stream " + std::to_string(in) + " does not exist");
		return *this;
	}
	auto& mapped = m_engine->Mapped;
	mapped.erase(std::remove_if(mapped.begin(), mapped.end(),
		[in](const Engine::Transcode::Slot& slot) { return slot.In == in; }),
		mapped.end());
	*m_logger << Level::Debug << "ignore stream " << in << std::endl;
	return *this;
}

Transcode& Transcode::Destination(const Container& container, std::filesystem::path path) noexcept {
	*m_logger << Level::LowLevel << "Transcode::Destination" << std::endl;
	if (!container.HasAccess(Operation::Write)) {
		Fail("container '" + std::string(container.Name()) + "' is not writable");
		return *this;
	}
	if (!path.empty())
		m_engine->Path = std::move(path);
	if (m_engine->Path.empty()) {
		Fail("destination path is empty");
		return *this;
	}
	m_engine->Container = &container;
	return *this;
}

void Transcode::Run() noexcept {
	if (m_engine)
		m_engine->Start(*this);
}

void Transcode::Cancel() noexcept {
	if (m_engine)
		m_engine->RequestCancel();
}

void Transcode::Pause() noexcept {
	if (!m_engine)
		return;
	if (m_engine->Status.load(std::memory_order_acquire) != Status::Running)
		return;
	m_engine->Paused.store(true, std::memory_order_release);
	m_engine->Status.store(Status::Paused, std::memory_order_release);
}

void Transcode::Resume() noexcept {
	if (!m_engine)
		return;
	if (m_engine->Status.load(std::memory_order_acquire) != Status::Paused)
		return;
	m_engine->Paused.store(false, std::memory_order_release);
	m_engine->Status.store(Status::Running, std::memory_order_release);
	m_engine->PauseCv.notify_all();
}

enum Status Transcode::Status() const noexcept {
	if (!m_engine)
		return Status::Error;
	return m_engine->Status.load(std::memory_order_acquire);
}

bool Transcode::Failed() const noexcept {
	return Status() == Status::Error;
}

std::optional<std::string> Transcode::Error() const noexcept {
	if (!m_engine)
		return std::nullopt;
	std::lock_guard lock(m_engine->Lock);
	return m_engine->Error;
}

std::optional<unsigned> Transcode::Progress() const noexcept {
	if (!m_engine || !m_engine->HasProgress.load(std::memory_order_acquire))
		return std::nullopt;
	return m_engine->Progress.load(std::memory_order_acquire);
}

Transcode::operator bool() const noexcept {
	const auto status = Status();
	return status != Status::Error && status != Status::Aborted;
}

std::unique_ptr<class Plan> Transcode::EmptyPlan(File&& source, const Container& container,
	std::filesystem::path destination) const noexcept {
	return std::make_unique<class Plan>(std::move(source), container, std::move(destination));
}

std::unique_ptr<TrackSettled> Transcode::EmptySettled() const noexcept {
	return std::make_unique<TrackSettled>();
}

void Transcode::OnConfigure() noexcept {}

enum Status Transcode::OnStart() noexcept {
	return Status::Running;
}

void Transcode::OnPlan(const class Plan&) noexcept {}

void Transcode::OnSettled(const TrackSettled&) noexcept {}

void Transcode::OnProgress(unsigned) noexcept {}

void Transcode::OnDone() noexcept {}

void Transcode::OnError(const std::string&) noexcept {}

void Transcode::OnAborted() noexcept {}

void Transcode::SetProgress(unsigned percent) noexcept {
	if (!m_engine)
		return;
	if (percent > 100)
		percent = 100;
	m_engine->Progress.store(percent, std::memory_order_release);
	m_engine->HasProgress.store(true, std::memory_order_release);
	OnProgress(percent);
}

void Transcode::MarkSettled(int in, Encoder& encoder) noexcept {
	if (!m_engine)
		return;
	for (auto& slot : m_engine->Mapped) {
		if (slot.In != in || slot.Settled)
			continue;
		slot.Settled = true;
		auto row = EmptySettled();
		row->In = slot.In;
		row->Out = slot.Out;
		row->Kind = slot.Kind;
		if (const Stream* stream = FindStream(Source(), slot.In))
			row->Source = &stream->Codec();
		if (const auto* video = AsVideo(slot.Config.get()))
			row->Destination = video->Codec();
		else if (const auto* audio = AsAudio(slot.Config.get()))
			row->Destination = audio->Codec();
		else if (const auto* subtitle = AsSubtitle(slot.Config.get()))
			row->Destination = subtitle->Codec();
		row->Implementation = slot.Config->Implementation().Encoder;
		if (encoder.Language())
			slot.Config->Language(*encoder.Language());
		if (encoder.Title())
			slot.Config->Title(*encoder.Title());
		OnSettled(*row);
		return;
	}
}
