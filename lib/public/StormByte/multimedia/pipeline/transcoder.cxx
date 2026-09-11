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

#include <StormByte/multimedia/backend/pipeline/transcoder.hxx>
#include <StormByte/multimedia/pipeline/transcoder.hxx>

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
using StormByte::Multimedia::ExpectedTranscoder;
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

Transcoder::Track::Track(Transcoder& owner, std::size_t slot) noexcept
: m_owner(&owner), m_slot(slot) {}

Transcoder::Track& Transcoder::Track::Remux() noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	*m_owner->m_logger << Level::Debug << "track "
		<< m_owner->m_backend->Mapped[m_slot].In << " marked remux" << std::endl;
	return *this;
}

Transcoder::Track& Transcoder::Track::Codec(const StormByte::Multimedia::Codec& codec) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	auto& slot = m_owner->m_backend->Mapped[m_slot];
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

Transcoder::Track& Transcoder::Track::Implementation(std::string name) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	auto& slot = m_owner->m_backend->Mapped[m_slot];
	auto impl = slot.Config->Implementation();
	impl.Encoder = std::move(name);
	slot.Config->Implementation(std::move(impl));
	return *this;
}

Transcoder::Track& Transcoder::Track::CRF(int value) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	if (auto* video = AsVideo(m_owner->m_backend->Mapped[m_slot].Config.get()))
		video->CRF(value);
	return *this;
}

Transcoder::Track& Transcoder::Track::BitRate(std::int64_t bits_per_second) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	auto* config = m_owner->m_backend->Mapped[m_slot].Config.get();
	if (auto* video = AsVideo(config))
		video->BitRate(bits_per_second);
	else if (auto* audio = AsAudio(config))
		audio->BitRate(bits_per_second);
	return *this;
}

Transcoder::Track& Transcoder::Track::MaxBitRate(std::int64_t bits_per_second) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	if (auto* audio = AsAudio(m_owner->m_backend->Mapped[m_slot].Config.get()))
		audio->MaxBitRate(bits_per_second);
	return *this;
}

Transcoder::Track& Transcoder::Track::Preset(std::string name) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	auto* config = m_owner->m_backend->Mapped[m_slot].Config.get();
	if (auto* video = AsVideo(config))
		video->Preset(std::move(name));
	else if (auto* audio = AsAudio(config))
		audio->Preset(std::move(name));
	return *this;
}

Transcoder::Track& Transcoder::Track::Tune(std::string name) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	if (auto* video = AsVideo(m_owner->m_backend->Mapped[m_slot].Config.get()))
		video->Tune(std::move(name));
	return *this;
}

Transcoder::Track& Transcoder::Track::FineTune(std::map<std::string, std::string> options) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	if (auto* video = AsVideo(m_owner->m_backend->Mapped[m_slot].Config.get()))
		video->FineTune(std::move(options));
	return *this;
}

Transcoder::Track& Transcoder::Track::Language(std::string language) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	m_owner->m_backend->Mapped[m_slot].Config->Language(std::move(language));
	return *this;
}

Transcoder::Track& Transcoder::Track::Title(std::string title) noexcept {
	if (!m_owner || !m_owner->ValidSlot(m_slot))
		return *this;
	m_owner->m_backend->Mapped[m_slot].Config->Title(std::move(title));
	return *this;
}

Transcoder::Transcoder(std::shared_ptr<StormByte::Logger::Log> logger, File&& file) noexcept
: m_logger(std::move(logger)), m_file(std::make_unique<File>(std::move(file))),
	m_backend(std::make_unique<Backend::Pipeline::Transcoder>()) {
	*m_logger << Level::LowLevel << "Transcoder::Transcoder" << std::endl;
}

Transcoder::~Transcoder() noexcept {
	if (m_logger)
		*m_logger << Level::LowLevel << "Transcoder::~Transcoder" << std::endl;
	if (!m_backend)
		return;
	const auto status = m_backend->Status.load(std::memory_order_acquire);
	if (status == Status::Running || status == Status::Paused)
		Cancel();
	m_backend->Join();
}

void Transcoder::Fail(std::string reason) noexcept {
	if (!m_backend)
		return;
	std::lock_guard lock(m_backend->Lock);
	if (m_backend->Status.load(std::memory_order_relaxed) == Status::Error)
		return;
	m_backend->Error = std::move(reason);
	m_backend->Status.store(Status::Error, std::memory_order_release);
	m_backend->RequestCancel();
	if (m_logger)
		*m_logger << Level::Error << *m_backend->Error << std::endl;
}

bool Transcoder::ValidSlot(std::size_t slot) const noexcept {
	return m_backend && slot < m_backend->Mapped.size();
}

void Transcoder::AttachFilter(std::size_t slot, std::shared_ptr<Filter::FFmpeg> filter) noexcept {
	if (!ValidSlot(slot) || !filter)
		return;
	if (dynamic_cast<Filter::Analytics*>(filter.get()) != nullptr) {
		Fail("Analytics attach on Transcoder::Filter, not Track::Filter");
		return;
	}
	m_backend->Mapped[slot].Filters.push_back(std::move(filter));
}

void Transcoder::AttachAnalytics(std::shared_ptr<Filter::FFmpeg> filter) noexcept {
	if (!m_backend || !filter)
		return;
	if (dynamic_cast<Filter::Analytics*>(filter.get()) == nullptr) {
		Fail("Track Process/Packet filters attach on Track::Filter");
		return;
	}
	m_backend->Analytics.push_back(std::move(filter));
}

ExpectedTranscoder Transcoder::BindLoggerAndFile(std::shared_ptr<StormByte::Logger::Log> logger,
	ExpectedFile opened) noexcept {
	if (!logger)
		return StormByte::Unexpected<TranscodeException>("logger is required");
	if (!opened) {
		const char* text = opened.error() ? opened.error()->what() : "file open failed";
		*logger << Level::Error << text << std::endl;
		return StormByte::Unexpected<TranscodeException>(text);
	}
	return std::unique_ptr<Transcoder>(new Transcoder(std::move(logger), std::move(*opened)));
}

ExpectedTranscoder Transcoder::Open(std::shared_ptr<StormByte::Logger::Log> logger,
	const std::filesystem::path& source, const std::filesystem::path& destination,
	std::optional<std::chrono::nanoseconds> duration) noexcept {
	if (!logger)
		return StormByte::Unexpected<TranscodeException>("logger is required");
	if (destination.empty())
		return StormByte::Unexpected<TranscodeException>("destination path is empty");
	ExpectedFile opened = duration
		? File::Open(source, *duration)
		: File::Open(source);
	auto job = BindLoggerAndFile(std::move(logger), std::move(opened));
	if (!job)
		return job;
	(*job)->m_backend->Path = destination;
	if (const auto& log = (*job)->m_logger)
		*log << Level::Notice << "opened source " << (*job)->Source().Path().string() << std::endl;
	return job;
}

const File& Transcoder::Source() const noexcept {
	if (m_plan)
		return m_plan->Source();
	return *m_file;
}

const std::shared_ptr<StormByte::Logger::Log>& Transcoder::Logger() const noexcept {
	return m_logger;
}

Transcoder::Track Transcoder::AddTrack(int in, int out, Type kind) noexcept {
	*m_logger << Level::LowLevel << "Transcoder::AddTrack in=" << in << " out=" << out << std::endl;
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
	for (const auto& slot : m_backend->Mapped) {
		if (slot.Out == out) {
			Fail("destination order key " + std::to_string(out) + " is already used");
			return Track(*this, InvalidSlot);
		}
	}
	Backend::Pipeline::TranscoderSlot slot;
	slot.In = in;
	slot.Out = out;
	slot.Kind = kind;
	if (kind == Type::Video)
		slot.Config = std::make_unique<Config::Video>();
	else if (kind == Type::Audio)
		slot.Config = std::make_unique<Config::Audio>();
	else
		slot.Config = std::make_unique<Config::Subtitle>();
	m_backend->Mapped.push_back(std::move(slot));
	*m_logger << Level::Debug << "mapped " << KindName(kind) << " " << in << " -> key " << out << std::endl;
	return Track(*this, m_backend->Mapped.size() - 1);
}

Transcoder::Track Transcoder::Video(int in, int out) noexcept {
	return AddTrack(in, out, Type::Video);
}

Transcoder::Track Transcoder::Audio(int in, int out) noexcept {
	return AddTrack(in, out, Type::Audio);
}

Transcoder::Track Transcoder::Subtitle(int in, int out) noexcept {
	return AddTrack(in, out, Type::Subtitle);
}

Transcoder& Transcoder::Ignore(int in) noexcept {
	*m_logger << Level::LowLevel << "Transcoder::Ignore " << in << std::endl;
	if (!FindStream(Source(), in)) {
		Fail("source stream " + std::to_string(in) + " does not exist");
		return *this;
	}
	auto& mapped = m_backend->Mapped;
	mapped.erase(std::remove_if(mapped.begin(), mapped.end(),
		[in](const Backend::Pipeline::TranscoderSlot& slot) { return slot.In == in; }),
		mapped.end());
	*m_logger << Level::Debug << "ignore stream " << in << std::endl;
	return *this;
}

Transcoder& Transcoder::Destination(const Container& container, std::filesystem::path path) noexcept {
	*m_logger << Level::LowLevel << "Transcoder::Destination" << std::endl;
	if (!container.HasAccess(Operation::Write)) {
		Fail("container '" + std::string(container.Name()) + "' is not writable");
		return *this;
	}
	if (!path.empty())
		m_backend->Path = std::move(path);
	if (m_backend->Path.empty()) {
		Fail("destination path is empty");
		return *this;
	}
	m_backend->Container = &container;
	return *this;
}

void Transcoder::Run() noexcept {
	if (m_backend)
		m_backend->Start(*this);
}

void Transcoder::Cancel() noexcept {
	if (m_backend)
		m_backend->RequestCancel();
}

void Transcoder::Pause() noexcept {
	if (!m_backend)
		return;
	if (m_backend->Status.load(std::memory_order_acquire) != Status::Running)
		return;
	m_backend->Paused.store(true, std::memory_order_release);
	m_backend->Status.store(Status::Paused, std::memory_order_release);
}

void Transcoder::Resume() noexcept {
	if (!m_backend)
		return;
	if (m_backend->Status.load(std::memory_order_acquire) != Status::Paused)
		return;
	m_backend->Paused.store(false, std::memory_order_release);
	m_backend->Status.store(Status::Running, std::memory_order_release);
	m_backend->PauseCv.notify_all();
}

enum Status Transcoder::Status() const noexcept {
	if (!m_backend)
		return Status::Error;
	return m_backend->Status.load(std::memory_order_acquire);
}

bool Transcoder::Failed() const noexcept {
	return Status() == Status::Error;
}

std::optional<std::string> Transcoder::Error() const noexcept {
	if (!m_backend)
		return std::nullopt;
	std::lock_guard lock(m_backend->Lock);
	return m_backend->Error;
}

std::optional<unsigned> Transcoder::Progress() const noexcept {
	if (!m_backend || !m_backend->HasProgress.load(std::memory_order_acquire))
		return std::nullopt;
	return m_backend->Progress.load(std::memory_order_acquire);
}

Transcoder::operator bool() const noexcept {
	const auto status = Status();
	return status != Status::Error && status != Status::Aborted;
}

std::unique_ptr<class Plan> Transcoder::EmptyPlan(File&& source, const Container& container,
	std::filesystem::path destination) const noexcept {
	return std::make_unique<class Plan>(std::move(source), container, std::move(destination));
}

std::unique_ptr<TrackSettled> Transcoder::EmptySettled() const noexcept {
	return std::make_unique<TrackSettled>();
}

void Transcoder::OnConfigure() noexcept {}

enum Status Transcoder::OnStart() noexcept {
	return Status::Running;
}

void Transcoder::OnPlan(const class Plan&) noexcept {}

void Transcoder::OnSettled(const TrackSettled&) noexcept {}

void Transcoder::OnProgress(unsigned) noexcept {}

void Transcoder::OnDone() noexcept {}

void Transcoder::OnError(const std::string&) noexcept {}

void Transcoder::OnAborted() noexcept {}

void Transcoder::SetProgress(unsigned percent) noexcept {
	if (!m_backend)
		return;
	if (percent > 100)
		percent = 100;
	m_backend->Progress.store(percent, std::memory_order_release);
	m_backend->HasProgress.store(true, std::memory_order_release);
	OnProgress(percent);
}

void Transcoder::MarkSettled(int in, Encoder& encoder) noexcept {
	if (!m_backend)
		return;
	for (auto& slot : m_backend->Mapped) {
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
		row->Implementation = encoder.Implementation();
		row->Crf = encoder.CRF();
		row->BitRate = encoder.BitRate();
		row->MaxBitRate = encoder.MaxBitRate();
		row->Preset = encoder.Preset();
		row->Tune = encoder.Tune();
		row->FineTune = encoder.FineTune();
		row->SampleFormat = encoder.AudioSampleFormat();
		row->EncoderChannels = encoder.AudioChannels();
		row->FrameSize = encoder.AudioFrameSize();
		row->SampleRate = encoder.AudioSampleRate();
		OnSettled(*row);
		return;
	}
}
