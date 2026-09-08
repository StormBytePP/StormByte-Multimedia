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
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/stream.hxx>

#include <algorithm>
#include <limits>
#include <utility>

using StormByte::Logger::Level;
using StormByte::Multimedia::Codec;
using StormByte::Multimedia::Container;
using StormByte::Multimedia::File;
using StormByte::Multimedia::TranscodeException;
using StormByte::Multimedia::Type;
using StormByte::Multimedia::Operation;

namespace StormByte::Multimedia::Pipeline {
	namespace {
		constexpr std::size_t InvalidSlot = std::numeric_limits<std::size_t>::max();

		const Stream* FindStream(const File& file, int index) noexcept {
			for (const auto& stream : file.Streams()) {
				if (stream.Index() == index)
					return &stream;
			}
			return nullptr;
		}

		std::string KindName(Type type) noexcept {
			return ToString(type);
		}
	}

	TrackPlan::PointerType TrackPlan::Clone() const {
		return MakePointer<TrackPlan>(*this);
	}

	TrackPlan::PointerType TrackPlan::Move() {
		return MakePointer<TrackPlan>(std::move(*this));
	}

	std::string TrackPlan::ToString() const {
		std::string text = "track in=" + std::to_string(in)
			+ " out=" + std::to_string(this->out);
		text += copy ? " copy" : " recode";
		if (source)
			text += std::string(" src=") + std::string(source->Name());
		if (destination)
			text += std::string(" dst=") + std::string(destination->Name());
		if (implementation)
			text += " impl=" + *implementation;
		if (language)
			text += " lang=" + *language;
		if (title)
			text += " title=" + *title;
		if (sourceChannels)
			text += " ch_in=" + std::to_string(*sourceChannels);
		if (encoderChannels)
			text += " ch_out=" + std::to_string(*encoderChannels);
		if (sampleFormat)
			text += " fmt=" + std::to_string(*sampleFormat);
		if (frameSize)
			text += " frame_size=" + std::to_string(*frameSize);
		if (sampleRate)
			text += " rate=" + std::to_string(*sampleRate);
		return text;
	}

	Plan::Plan(const Plan& other)
	: source(other.source), container(other.container), destination(other.destination),
	ignored(other.ignored), videoPacketCeiling(other.videoPacketCeiling),
	muxPacketCeiling(other.muxPacketCeiling), copyPacketCeiling(other.copyPacketCeiling),
	videoFrameCeiling(other.videoFrameCeiling) {
		tracks.reserve(other.tracks.size());
		for (const auto& track : other.tracks) {
			if (track)
				tracks.push_back(track->Clone());
		}
	}

	Plan& Plan::operator=(const Plan& other) {
		if (this == &other)
			return *this;
		source = other.source;
		container = other.container;
		destination = other.destination;
		ignored = other.ignored;
		videoPacketCeiling = other.videoPacketCeiling;
		muxPacketCeiling = other.muxPacketCeiling;
		copyPacketCeiling = other.copyPacketCeiling;
		videoFrameCeiling = other.videoFrameCeiling;
		tracks.clear();
		tracks.reserve(other.tracks.size());
		for (const auto& track : other.tracks) {
			if (track)
				tracks.push_back(track->Clone());
		}
		return *this;
	}

	Plan::PointerType Plan::Clone() const {
		return MakePointer<Plan>(*this);
	}

	Plan::PointerType Plan::Move() {
		return MakePointer<Plan>(std::move(*this));
	}

	std::string Plan::ToString() const {
		std::string text = "plan dest=" + destination.string();
		if (container)
			text += std::string(" container=") + std::string(container->Name());
		text += " tracks=" + std::to_string(tracks.size());
		text += " ignore=" + std::to_string(ignored.size());
		text += "\n";
		for (const auto& track : tracks) {
			if (track)
				text += "  " + track->ToString() + "\n";
		}
		return text;
	}

	Transcode::Track::Track(Transcode& owner, std::size_t slot) noexcept
	: m_owner(&owner), m_slot(slot) {}

	Transcode::Track& Transcode::Track::Copy() noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		auto& slot = m_owner->m_engine->mapped[m_slot];
		slot.copy = true;
		slot.codec = nullptr;
		*m_owner->m_logger << Level::Debug << "track " << slot.in << " marked copy" << std::endl;
		return *this;
	}

	Transcode::Track& Transcode::Track::Codec(const StormByte::Multimedia::Codec& codec) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		auto& slot = m_owner->m_engine->mapped[m_slot];
		if (codec.Type() != slot.kind) {
			m_owner->Fail("codec '" + std::string(codec.Name()) + "' is "
				+ KindName(codec.Type()) + ", track " + std::to_string(slot.in)
				+ " is " + KindName(slot.kind));
			return *this;
		}
		slot.codec = &codec;
		slot.copy = false;
		*m_owner->m_logger << Level::Debug << "track " << slot.in << " recode to "
			<< std::string(codec.Name()) << std::endl;
		return *this;
	}

	Transcode::Track& Transcode::Track::Implementation(std::string name) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_engine->mapped[m_slot].implementation = std::move(name);
		return *this;
	}

	Transcode::Track& Transcode::Track::CRF(int value) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		auto& slot = m_owner->m_engine->mapped[m_slot];
		slot.crf = value;
		slot.bitRate.reset();
		return *this;
	}

	Transcode::Track& Transcode::Track::BitRate(std::int64_t bits_per_second) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		auto& slot = m_owner->m_engine->mapped[m_slot];
		slot.bitRate = bits_per_second;
		slot.crf.reset();
		return *this;
	}

	Transcode::Track& Transcode::Track::MaxBitRate(std::int64_t bits_per_second) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_engine->mapped[m_slot].maxBitRate = bits_per_second;
		return *this;
	}

	Transcode::Track& Transcode::Track::Preset(std::string name) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_engine->mapped[m_slot].preset = std::move(name);
		return *this;
	}

	Transcode::Track& Transcode::Track::Tune(std::string name) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_engine->mapped[m_slot].tune = std::move(name);
		return *this;
	}

	Transcode::Track& Transcode::Track::FineTune(std::map<std::string, std::string> options) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_engine->mapped[m_slot].fineTune = std::move(options);
		return *this;
	}

	Transcode::Track& Transcode::Track::Language(std::string language) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_engine->mapped[m_slot].language = std::move(language);
		return *this;
	}

	Transcode::Track& Transcode::Track::Title(std::string title) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_engine->mapped[m_slot].title = std::move(title);
		return *this;
	}

	Transcode::Transcode(std::shared_ptr<StormByte::Logger::Log> logger, File file) noexcept
	: m_logger(std::move(logger)), m_file(std::make_unique<File>(std::move(file))),
	m_engine(std::make_unique<Engine::Transcode::Engine>()),
	m_pipe(std::make_shared<Filter::Chain>()) {
		*m_logger << Level::LowLevel << "Transcode::Transcode" << std::endl;
	}

	Transcode::Transcode(Transcode&& other) noexcept
	: m_logger(std::move(other.m_logger)), m_file(std::move(other.m_file)),
	m_engine(std::move(other.m_engine)), m_pipe(std::move(other.m_pipe)) {
		if (m_logger)
			*m_logger << Level::LowLevel << "Transcode::Transcode(move)" << std::endl;
	}

	Transcode::~Transcode() noexcept {
		if (m_logger)
			*m_logger << Level::LowLevel << "Transcode::~Transcode" << std::endl;
		if (!m_engine)
			return;
		const auto status = m_engine->status.load(std::memory_order_acquire);
		if (status == Status::Running || status == Status::Paused)
			Cancel();
		m_engine->Join();
	}

	Transcode& Transcode::operator=(Transcode&& other) noexcept {
		if (this == &other)
			return *this;
		if (m_engine) {
			const auto status = m_engine->status.load(std::memory_order_acquire);
			if (status == Status::Running || status == Status::Paused)
				Cancel();
			m_engine->Join();
		}
		m_logger = std::move(other.m_logger);
		m_file = std::move(other.m_file);
		m_engine = std::move(other.m_engine);
		m_pipe = std::move(other.m_pipe);
		return *this;
	}

	void Transcode::Fail(std::string reason) noexcept {
		if (!m_engine)
			return;
		std::lock_guard lock(m_engine->lock);
		if (m_engine->status.load(std::memory_order_relaxed) == Status::Error)
			return;
		m_engine->error = std::move(reason);
		m_engine->status.store(Status::Error, std::memory_order_release);
		m_engine->RequestCancel();
		if (m_logger)
			*m_logger << Level::Error << *m_engine->error << std::endl;
	}

	bool Transcode::ValidSlot(std::size_t slot) const noexcept {
		return m_engine && slot < m_engine->mapped.size();
	}

	ExpectedTranscode Transcode::BindLoggerAndFile(std::shared_ptr<StormByte::Logger::Log> logger,
		ExpectedFile opened) noexcept {
		if (!logger)
			return StormByte::Unexpected<TranscodeException>("logger is required");
		if (!opened) {
			const auto& err = opened.error();
			const char* text = err ? err->what() : "file open failed";
			*logger << Level::Error << text << std::endl;
			return StormByte::Unexpected<TranscodeException>(text);
		}
		File file = std::move(*opened);
		(void)file.Duration();
		*logger << Level::Notice << "opened source"
			<< (file.Path().empty() ? std::string(" buffer") : std::string(" ") + file.Path().string())
			<< std::endl;
		return std::unique_ptr<Transcode>(new Transcode(std::move(logger), std::move(file)));
	}

	ExpectedTranscode Transcode::Open(std::shared_ptr<StormByte::Logger::Log> logger,
		const std::filesystem::path& path) noexcept {
		if (logger)
			*logger << Level::LowLevel << "Transcode::Open path" << std::endl;
		return BindLoggerAndFile(std::move(logger), File::Open(path));
	}

	ExpectedTranscode Transcode::Open(std::shared_ptr<StormByte::Logger::Log> logger,
		const std::filesystem::path& path, std::chrono::nanoseconds duration) noexcept {
		if (logger)
			*logger << Level::LowLevel << "Transcode::Open path+duration" << std::endl;
		return BindLoggerAndFile(std::move(logger), File::Open(path, duration));
	}

	ExpectedTranscode Transcode::Open(std::shared_ptr<StormByte::Logger::Log> logger,
		StormByte::Buffer::Consumer consumer) noexcept {
		if (logger)
			*logger << Level::LowLevel << "Transcode::Open consumer" << std::endl;
		return BindLoggerAndFile(std::move(logger), File::Open(std::move(consumer)));
	}

	ExpectedTranscode Transcode::Open(std::shared_ptr<StormByte::Logger::Log> logger,
		StormByte::Buffer::Consumer consumer, std::chrono::nanoseconds duration) noexcept {
		if (logger)
			*logger << Level::LowLevel << "Transcode::Open consumer+duration" << std::endl;
		return BindLoggerAndFile(std::move(logger), File::Open(std::move(consumer), duration));
	}

	const File& Transcode::Source() const noexcept {
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
		const Stream* stream = FindStream(*m_file, in);
		if (!stream) {
			Fail("source stream " + std::to_string(in) + " does not exist");
			return Track(*this, InvalidSlot);
		}
		if (stream->Type() != kind) {
			Fail("stream " + std::to_string(in) + " is " + KindName(stream->Type())
				+ ", " + KindName(kind) + "() requires " + KindName(kind));
			return Track(*this, InvalidSlot);
		}
		for (const auto& slot : m_engine->mapped) {
			if (slot.outKey == out) {
				Fail("destination order key " + std::to_string(out) + " is already used");
				return Track(*this, InvalidSlot);
			}
			if (slot.in == in) {
				Fail("source stream " + std::to_string(in) + " is already mapped");
				return Track(*this, InvalidSlot);
			}
		}
		Engine::Transcode::Slot slot;
		slot.in = in;
		slot.outKey = out;
		slot.kind = kind;
		m_engine->mapped.push_back(std::move(slot));
		m_engine->ignore.erase(in);
		*m_logger << Level::Debug << "mapped " << KindName(kind) << " " << in << " -> key " << out << std::endl;
		return Track(*this, m_engine->mapped.size() - 1);
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
		if (!FindStream(*m_file, in)) {
			Fail("source stream " + std::to_string(in) + " does not exist");
			return *this;
		}
		m_engine->mapped.erase(std::remove_if(m_engine->mapped.begin(), m_engine->mapped.end(),
			[in](const Engine::Transcode::Slot& slot) { return slot.in == in; }),
			m_engine->mapped.end());
		m_engine->ignore.insert(in);
		*m_logger << Level::Debug << "ignore stream " << in << std::endl;
		return *this;
	}

	Transcode& Transcode::Destination(const Container& container, std::filesystem::path path) noexcept {
		*m_logger << Level::LowLevel << "Transcode::Destination" << std::endl;
		if (!container.HasAccess(Operation::Write)) {
			Fail("container '" + std::string(container.Name()) + "' is not writable");
			return *this;
		}
		m_engine->container = &container;
		m_engine->path = std::move(path);
		*m_logger << Level::Notice << "destination " << m_engine->path.string()
			<< " container " << std::string(container.Name()) << std::endl;
		return *this;
	}

	void Transcode::Run() noexcept {
		*m_logger << Level::LowLevel << "Transcode::Run" << std::endl;
		if (!m_engine)
			return;
		const auto status = m_engine->status.load(std::memory_order_acquire);
		if (status == Status::Running || status == Status::Paused) {
			Fail("transcode is already running");
			return;
		}
		m_engine->Join();
		m_engine->cancel.store(false, std::memory_order_release);
		m_engine->paused.store(false, std::memory_order_release);
		m_engine->muxQueue.reset();
		m_engine->copyQueue.reset();
		m_engine->encodeQueues.clear();
		m_engine->frameQueues.clear();
		{
			std::lock_guard lock(m_engine->lock);
			m_engine->error.reset();
			m_engine->progress.store(0, std::memory_order_relaxed);
			m_engine->hasProgress.store(false, std::memory_order_relaxed);
			m_engine->status.store(Status::Running, std::memory_order_release);
		}
		m_engine->worker = std::thread([this]() { m_engine->Run(*this); });
	}

	void Transcode::Cancel() noexcept {
		if (!m_engine)
			return;
		*m_logger << Level::LowLevel << "Transcode::Cancel" << std::endl;
		const auto status = m_engine->status.load(std::memory_order_acquire);
		if (status != Status::Running && status != Status::Paused)
			return;
		m_engine->RequestCancel();
		*m_logger << Level::Notice << "cancel requested" << std::endl;
	}

	void Transcode::Pause() noexcept {
		if (!m_engine)
			return;
		*m_logger << Level::LowLevel << "Transcode::Pause" << std::endl;
		auto expected = Status::Running;
		if (!m_engine->status.compare_exchange_strong(expected, Status::Paused,
			std::memory_order_acq_rel))
			return;
		m_engine->paused.store(true, std::memory_order_release);
		*m_logger << Level::Notice << "paused" << std::endl;
	}

	void Transcode::Resume() noexcept {
		if (!m_engine)
			return;
		*m_logger << Level::LowLevel << "Transcode::Resume" << std::endl;
		auto expected = Status::Paused;
		if (!m_engine->status.compare_exchange_strong(expected, Status::Running,
			std::memory_order_acq_rel))
			return;
		m_engine->paused.store(false, std::memory_order_release);
		m_engine->pauseCv.notify_all();
		*m_logger << Level::Notice << "resumed" << std::endl;
	}

	enum Status Transcode::Status() const noexcept {
		if (!m_engine)
			return Status::Error;
		return m_engine->status.load(std::memory_order_acquire);
	}

	bool Transcode::Failed() const noexcept {
		return Status() == Status::Error;
	}

	std::optional<std::string> Transcode::Error() const noexcept {
		if (!m_engine)
			return std::nullopt;
		std::lock_guard lock(m_engine->lock);
		return m_engine->error;
	}

	std::optional<unsigned> Transcode::Progress() const noexcept {
		if (!m_engine || !m_engine->hasProgress.load(std::memory_order_acquire))
			return std::nullopt;
		const auto status = Status();
		if (status == Status::Stopped)
			return std::nullopt;
		return m_engine->progress.load(std::memory_order_acquire);
	}

	Transcode::operator bool() const noexcept {
		const auto status = Status();
		return status == Status::Stopped || status == Status::Running
			|| status == Status::Paused || status == Status::Done;
	}

	std::unique_ptr<Plan> Transcode::MakePlan() const noexcept {
		return Plan::MakePointer<Plan>();
	}

	std::unique_ptr<TrackPlan> Transcode::MakeTrackPlan() const noexcept {
		return TrackPlan::MakePointer<TrackPlan>();
	}

	std::unique_ptr<Plan> Transcode::Configuration() const noexcept {
		auto plan = MakePlan();
		if (!plan)
			return nullptr;
		plan->source = m_file.get();
		if (m_engine) {
			plan->container = m_engine->container;
			plan->destination = m_engine->path;
			plan->ignored.assign(m_engine->ignore.begin(), m_engine->ignore.end());
		}
		plan->videoPacketCeiling = m_videoPacketCeiling;
		plan->muxPacketCeiling = m_muxPacketCeiling;
		plan->copyPacketCeiling = m_copyPacketCeiling;
		plan->videoFrameCeiling = m_videoFrameCeiling;
		if (!m_engine)
			return plan;
		for (const auto& slot : m_engine->mapped) {
			auto row = MakeTrackPlan();
			if (!row)
				continue;
			row->in = slot.in;
			row->out = slot.outKey;
			row->kind = slot.kind;
			row->copy = slot.copy;
			row->destination = slot.codec;
			row->implementation = slot.implementation;
			row->language = slot.language;
			row->title = slot.title;
			row->crf = slot.crf;
			row->bitRate = slot.bitRate;
			row->maxBitRate = slot.maxBitRate;
			row->preset = slot.preset;
			row->tune = slot.tune;
			row->fineTune = slot.fineTune;
			row->sampleFormat = slot.sampleFormat;
			row->encoderChannels = slot.encoderChannels;
			row->frameSize = slot.frameSize;
			if (m_file) {
				for (const auto& stream : m_file->Streams()) {
					if (stream.Index() != slot.in)
						continue;
					row->source = &stream.Codec();
					if (stream.Audio()) {
						row->sourceChannels = static_cast<int>(stream.Audio()->Channels());
						row->sampleRate = static_cast<int>(stream.Audio()->SampleRate());
					}
					break;
				}
			}
			if (slot.settledRate)
				row->sampleRate = slot.settledRate;
			plan->tracks.push_back(std::move(row));
		}
		return plan;
	}

	void Transcode::MarkSettled(int in, Encoder& encoder) noexcept {
		if (!m_engine || !encoder.Opened())
			return;
		for (auto& slot : m_engine->mapped) {
			if (slot.in != in || slot.settled)
				continue;
			slot.implementation = encoder.Implementation();
			slot.sampleFormat = encoder.AudioSampleFormat();
			slot.encoderChannels = encoder.AudioChannels();
			slot.frameSize = encoder.AudioFrameSize();
			slot.settledRate = encoder.AudioSampleRate();
			slot.settled = true;
			if (auto row = MakeTrackPlan()) {
				row->in = slot.in;
				row->out = slot.outKey;
				row->kind = slot.kind;
				row->copy = false;
				row->destination = slot.codec;
				row->implementation = slot.implementation;
				row->sampleFormat = slot.sampleFormat;
				row->encoderChannels = slot.encoderChannels;
				row->frameSize = slot.frameSize;
				row->sampleRate = slot.settledRate;
				OnSettled(*row);
			}
			break;
		}
	}

	void Transcode::OnConfigure() noexcept {}

	enum Status Transcode::OnStart() noexcept {
		return Status::Running;
	}

	void Transcode::OnPlan(const Plan& plan) noexcept {
		if (m_logger)
			*m_logger << Level::LowLevel << plan.ToString() << std::endl;
	}

	void Transcode::OnSettled(const TrackPlan& track) noexcept {
		if (m_logger)
			*m_logger << Level::LowLevel << "settled " << track.ToString() << std::endl;
	}

	void Transcode::OnProgress(unsigned) noexcept {}
	void Transcode::OnDone() noexcept {}
	void Transcode::OnError(const std::string&) noexcept {}
	void Transcode::OnAborted() noexcept {}

	void Transcode::SetProgress(unsigned percent) noexcept {
		if (percent > 100)
			percent = 100;
		m_engine->progress.store(percent, std::memory_order_release);
		m_engine->hasProgress.store(true, std::memory_order_release);
		OnProgress(percent);
	}

	void Transcode::Worker() noexcept {
		if (m_engine)
			m_engine->Run(*this);
	}
}
