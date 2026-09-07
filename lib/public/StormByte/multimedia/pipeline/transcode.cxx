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

#include <StormByte/expected.hxx>
#include <StormByte/logger/typedefs.hxx>
#include <StormByte/multimedia/pipeline/copy.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/mux.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/stream.hxx>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <limits>
#include <mutex>
#include <set>
#include <thread>
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

		std::string FormatElapsed(std::chrono::steady_clock::time_point start) noexcept {
			const auto total = std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::steady_clock::now() - start).count();
			const auto hours = total / 3600;
			const auto minutes = (total % 3600) / 60;
			const auto seconds = total % 60;
			char buffer[16];
			if (hours > 0)
				std::snprintf(buffer, sizeof(buffer), "%lld:%02lld:%02lld",
					static_cast<long long>(hours),
					static_cast<long long>(minutes),
					static_cast<long long>(seconds));
			else if (minutes > 0)
				std::snprintf(buffer, sizeof(buffer), "%lld:%02lld",
					static_cast<long long>(minutes),
					static_cast<long long>(seconds));
			else
				std::snprintf(buffer, sizeof(buffer), "%lld",
					static_cast<long long>(seconds));
			return buffer;
		}

		std::size_t AtLeastOne(std::size_t value) noexcept {
			return value == 0 ? 1 : value;
		}

		template<typename Item>
		class BoundQueue {
			public:
				explicit BoundQueue(std::size_t ceiling) noexcept
				: m_ceiling(AtLeastOne(ceiling)) {}

				void Push(Item item, const std::atomic_bool& cancel) noexcept {
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

				bool TryPush(Item& item) noexcept {
					std::lock_guard lock(m_mutex);
					if (m_closed || m_queue.size() >= m_ceiling)
						return false;
					m_queue.push_back(std::move(item));
					m_cv.notify_one();
					return true;
				}

				std::optional<Item> Pop(const std::atomic_bool& cancel) noexcept {
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

				std::optional<Item> TryPop() noexcept {
					std::lock_guard lock(m_mutex);
					if (m_queue.empty())
						return std::nullopt;
					Item item = std::move(m_queue.front());
					m_queue.pop_front();
					m_cv.notify_all();
					return item;
				}

				bool Empty() const noexcept {
					std::lock_guard lock(m_mutex);
					return m_queue.empty();
				}

				bool Full() const noexcept {
					std::lock_guard lock(m_mutex);
					return m_queue.size() >= m_ceiling;
				}

				std::size_t Size() const noexcept {
					std::lock_guard lock(m_mutex);
					return m_queue.size();
				}

				bool Closed() const noexcept {
					std::lock_guard lock(m_mutex);
					return m_closed && m_queue.empty();
				}

				void Close() noexcept {
					{
						std::lock_guard lock(m_mutex);
						m_closed = true;
					}
					m_cv.notify_all();
				}

				void Wake() noexcept {
					m_cv.notify_all();
				}

			private:
				std::size_t m_ceiling;
				std::deque<Item> m_queue;
				mutable std::mutex m_mutex;
				std::condition_variable m_cv;
				bool m_closed = false;
		};

		using PacketQueue = BoundQueue<Packet>;
		using FrameQueue = BoundQueue<Frame>;
	}

	struct Transcode::Slot {
		int in = -1;
		int outKey = -1;
		Type kind = Type::Video;
		bool copy = false;
		const Codec* codec = nullptr;
		std::optional<std::string> implementation;
		std::optional<int> crf;
		std::optional<std::int64_t> bitRate;
		std::optional<std::int64_t> maxBitRate;
		std::optional<std::string> preset;
		std::optional<std::string> tune;
		std::map<std::string, std::string> fineTune;
		std::optional<std::string> language;
		std::optional<std::string> title;
	};

	class Transcode::Impl {
		public:
			Impl() noexcept = default;

			~Impl() noexcept {
				RequestCancel();
				Join();
			}

			void NotifyIntake() noexcept {
				m_intakeCv.notify_all();
			}

			void WaitForIntake() noexcept {
				std::unique_lock lock(m_intakeMutex);
				m_intakeCv.wait(lock, [&]() {
					return m_cancel.load(std::memory_order_acquire)
						|| (m_muxQueue && !m_muxQueue->Empty())
						|| (m_copyQueue && !m_copyQueue->Empty())
						|| ((m_muxQueue && m_muxQueue->Closed())
							&& (m_copyQueue && m_copyQueue->Closed()));
				});
			}

			void NotifySpace() noexcept {
				m_spaceCv.notify_all();
			}

			void WaitForMuxSpace() noexcept {
				std::unique_lock lock(m_spaceMutex);
				m_spaceCv.wait(lock, [&]() {
					return m_cancel.load(std::memory_order_acquire)
						|| !m_muxQueue
						|| !m_muxQueue->Full();
				});
			}

			void RequestCancel() noexcept {
				m_cancel.store(true, std::memory_order_release);
				m_paused.store(false, std::memory_order_release);
				m_pauseCv.notify_all();
				NotifyIntake();
				NotifySpace();
				if (m_muxQueue)
					m_muxQueue->Wake();
				if (m_copyQueue)
					m_copyQueue->Wake();
				for (auto& queue : m_encodeQueues) {
					if (queue)
						queue->Wake();
				}
				for (auto& queue : m_frameQueues) {
					if (queue)
						queue->Wake();
				}
			}

			void Join() noexcept {
				if (m_worker.joinable())
					m_worker.join();
			}

			void WaitIfPaused() noexcept {
				std::unique_lock lock(m_pauseMutex);
				m_pauseCv.wait(lock, [&]() {
					return m_cancel.load(std::memory_order_acquire)
						|| !m_paused.load(std::memory_order_acquire);
				});
			}

			mutable std::mutex m_lock;
			std::mutex m_pauseMutex;
			std::condition_variable m_pauseCv;
			std::mutex m_intakeMutex;
			std::condition_variable m_intakeCv;
			std::mutex m_spaceMutex;
			std::condition_variable m_spaceCv;
			std::atomic<enum Status> m_status { Status::Stopped };
			std::atomic_bool m_cancel { false };
			std::atomic_bool m_paused { false };
			std::atomic<unsigned> m_progress { 0 };
			std::atomic_bool m_hasProgress { false };
			std::atomic<std::size_t> m_heldSize { 0 };
			std::atomic<std::size_t> m_sendFrames { 0 };
			std::atomic<std::size_t> m_recvPackets { 0 };
			std::atomic<std::size_t> m_muxBlocked { 0 };
			std::atomic<std::size_t> m_copyBlocked { 0 };
			std::optional<std::string> m_error;
			const Container* m_container = nullptr;
			std::filesystem::path m_path;
			std::vector<Slot> m_explicit;
			std::set<int> m_ignore;
			std::thread m_worker;
			std::unique_ptr<PacketQueue> m_muxQueue;
			std::unique_ptr<PacketQueue> m_copyQueue;
			std::vector<std::unique_ptr<PacketQueue>> m_encodeQueues;
			std::vector<std::unique_ptr<FrameQueue>> m_frameQueues;
	};

	Transcode::Track::Track(Transcode& owner, std::size_t slot) noexcept
	: m_owner(&owner), m_slot(slot) {}

	Transcode::Track& Transcode::Track::Copy() noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		auto& slot = m_owner->m_impl->m_explicit[m_slot];
		slot.copy = true;
		slot.codec = nullptr;
		*m_owner->m_logger << Level::Debug << "track " << slot.in << " marked copy" << std::endl;
		return *this;
	}

	Transcode::Track& Transcode::Track::Codec(const StormByte::Multimedia::Codec& codec) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		auto& slot = m_owner->m_impl->m_explicit[m_slot];
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
		m_owner->m_impl->m_explicit[m_slot].implementation = std::move(name);
		return *this;
	}

	Transcode::Track& Transcode::Track::CRF(int value) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		auto& slot = m_owner->m_impl->m_explicit[m_slot];
		slot.crf = value;
		slot.bitRate.reset();
		return *this;
	}

	Transcode::Track& Transcode::Track::BitRate(std::int64_t bits_per_second) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		auto& slot = m_owner->m_impl->m_explicit[m_slot];
		slot.bitRate = bits_per_second;
		slot.crf.reset();
		return *this;
	}

	Transcode::Track& Transcode::Track::MaxBitRate(std::int64_t bits_per_second) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_impl->m_explicit[m_slot].maxBitRate = bits_per_second;
		return *this;
	}

	Transcode::Track& Transcode::Track::Preset(std::string name) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_impl->m_explicit[m_slot].preset = std::move(name);
		return *this;
	}

	Transcode::Track& Transcode::Track::Tune(std::string name) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_impl->m_explicit[m_slot].tune = std::move(name);
		return *this;
	}

	Transcode::Track& Transcode::Track::FineTune(std::map<std::string, std::string> options) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_impl->m_explicit[m_slot].fineTune = std::move(options);
		return *this;
	}

	Transcode::Track& Transcode::Track::Language(std::string language) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_impl->m_explicit[m_slot].language = std::move(language);
		return *this;
	}

	Transcode::Track& Transcode::Track::Title(std::string title) noexcept {
		if (!m_owner || !m_owner->ValidSlot(m_slot))
			return *this;
		m_owner->m_impl->m_explicit[m_slot].title = std::move(title);
		return *this;
	}

	Transcode::Transcode(std::shared_ptr<StormByte::Logger::Log> logger, File file) noexcept
	: m_logger(std::move(logger)), m_file(std::make_unique<File>(std::move(file))),
	m_impl(std::make_unique<Impl>()) {
		*m_logger << Level::LowLevel << "Transcode::Transcode" << std::endl;
	}

	Transcode::Transcode(Transcode&& other) noexcept
	: m_logger(std::move(other.m_logger)), m_file(std::move(other.m_file)), m_impl(std::move(other.m_impl)) {
		if (m_logger)
			*m_logger << Level::LowLevel << "Transcode::Transcode(move)" << std::endl;
	}

	Transcode::~Transcode() noexcept {
		if (m_logger)
			*m_logger << Level::LowLevel << "Transcode::~Transcode" << std::endl;
		if (!m_impl)
			return;
		const auto status = m_impl->m_status.load(std::memory_order_acquire);
		if (status == Status::Running || status == Status::Paused)
			Cancel();
		m_impl->Join();
	}

	Transcode& Transcode::operator=(Transcode&& other) noexcept {
		if (this == &other)
			return *this;
		if (m_impl) {
			const auto status = m_impl->m_status.load(std::memory_order_acquire);
			if (status == Status::Running || status == Status::Paused)
				Cancel();
			m_impl->Join();
		}
		m_logger = std::move(other.m_logger);
		m_file = std::move(other.m_file);
		m_impl = std::move(other.m_impl);
		return *this;
	}

	void Transcode::Fail(std::string reason) noexcept {
		if (!m_impl)
			return;
		std::lock_guard lock(m_impl->m_lock);
		if (m_impl->m_status.load(std::memory_order_relaxed) == Status::Error)
			return;
		m_impl->m_error = std::move(reason);
		m_impl->m_status.store(Status::Error, std::memory_order_release);
		m_impl->RequestCancel();
		if (m_logger)
			*m_logger << Level::Error << *m_impl->m_error << std::endl;
	}

	bool Transcode::ValidSlot(std::size_t slot) const noexcept {
		return m_impl && slot < m_impl->m_explicit.size();
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
		for (const auto& slot : m_impl->m_explicit) {
			if (slot.outKey == out) {
				Fail("destination order key " + std::to_string(out) + " is already used");
				return Track(*this, InvalidSlot);
			}
			if (slot.in == in) {
				Fail("source stream " + std::to_string(in) + " is already mapped");
				return Track(*this, InvalidSlot);
			}
		}
		Slot slot;
		slot.in = in;
		slot.outKey = out;
		slot.kind = kind;
		m_impl->m_explicit.push_back(std::move(slot));
		m_impl->m_ignore.erase(in);
		*m_logger << Level::Debug << "mapped " << KindName(kind) << " " << in << " -> key " << out << std::endl;
		return Track(*this, m_impl->m_explicit.size() - 1);
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
		m_impl->m_explicit.erase(std::remove_if(m_impl->m_explicit.begin(), m_impl->m_explicit.end(),
			[in](const Slot& slot) { return slot.in == in; }), m_impl->m_explicit.end());
		m_impl->m_ignore.insert(in);
		*m_logger << Level::Debug << "ignore stream " << in << std::endl;
		return *this;
	}

	Transcode& Transcode::Destination(const Container& container, std::filesystem::path path) noexcept {
		*m_logger << Level::LowLevel << "Transcode::Destination" << std::endl;
		if (!container.HasAccess(Operation::Write)) {
			Fail("container '" + std::string(container.Name()) + "' is not writable");
			return *this;
		}
		m_impl->m_container = &container;
		m_impl->m_path = std::move(path);
		*m_logger << Level::Notice << "destination " << m_impl->m_path.string()
			<< " container " << std::string(container.Name()) << std::endl;
		return *this;
	}

	void Transcode::Run() noexcept {
		*m_logger << Level::LowLevel << "Transcode::Run" << std::endl;
		if (!m_impl)
			return;
		const auto status = m_impl->m_status.load(std::memory_order_acquire);
		if (status == Status::Running || status == Status::Paused) {
			Fail("transcode is already running");
			return;
		}
		m_impl->Join();
		m_impl->m_cancel.store(false, std::memory_order_release);
		m_impl->m_paused.store(false, std::memory_order_release);
		m_impl->m_muxQueue.reset();
		m_impl->m_copyQueue.reset();
		m_impl->m_encodeQueues.clear();
		m_impl->m_frameQueues.clear();
		{
			std::lock_guard lock(m_impl->m_lock);
			m_impl->m_error.reset();
			m_impl->m_progress.store(0, std::memory_order_relaxed);
			m_impl->m_hasProgress.store(true, std::memory_order_release);
			m_impl->m_status.store(Status::Running, std::memory_order_release);
		}
		OnProgress(0);
		m_impl->m_worker = std::thread([this]() { Worker(); });
	}

	void Transcode::Cancel() noexcept {
		if (!m_impl)
			return;
		*m_logger << Level::LowLevel << "Transcode::Cancel" << std::endl;
		const auto status = m_impl->m_status.load(std::memory_order_acquire);
		if (status != Status::Running && status != Status::Paused)
			return;
		m_impl->RequestCancel();
		*m_logger << Level::Notice << "cancel requested" << std::endl;
	}

	void Transcode::Pause() noexcept {
		if (!m_impl)
			return;
		*m_logger << Level::LowLevel << "Transcode::Pause" << std::endl;
		auto expected = Status::Running;
		if (!m_impl->m_status.compare_exchange_strong(expected, Status::Paused,
			std::memory_order_acq_rel))
			return;
		m_impl->m_paused.store(true, std::memory_order_release);
		*m_logger << Level::Notice << "paused" << std::endl;
	}

	void Transcode::Resume() noexcept {
		if (!m_impl)
			return;
		*m_logger << Level::LowLevel << "Transcode::Resume" << std::endl;
		auto expected = Status::Paused;
		if (!m_impl->m_status.compare_exchange_strong(expected, Status::Running,
			std::memory_order_acq_rel))
			return;
		m_impl->m_paused.store(false, std::memory_order_release);
		m_impl->m_pauseCv.notify_all();
		*m_logger << Level::Notice << "resumed" << std::endl;
	}

	Status Transcode::Status() const noexcept {
		if (!m_impl)
			return Status::Error;
		return m_impl->m_status.load(std::memory_order_acquire);
	}

	bool Transcode::Failed() const noexcept {
		return Status() == Status::Error;
	}

	std::optional<std::string> Transcode::Error() const noexcept {
		if (!m_impl)
			return std::nullopt;
		std::lock_guard lock(m_impl->m_lock);
		return m_impl->m_error;
	}

	std::optional<unsigned> Transcode::Progress() const noexcept {
		if (!m_impl || !m_impl->m_hasProgress.load(std::memory_order_acquire))
			return std::nullopt;
		const auto status = Status();
		if (status == Status::Stopped)
			return std::nullopt;
		return m_impl->m_progress.load(std::memory_order_acquire);
	}

	Transcode::operator bool() const noexcept {
		const auto status = Status();
		return status == Status::Stopped || status == Status::Running
			|| status == Status::Paused || status == Status::Done;
	}

	void Transcode::OnConfigure() noexcept {}
	Status Transcode::OnStart() noexcept { return Status::Running; }
	void Transcode::OnProgress(unsigned) noexcept {}
	void Transcode::OnDone() noexcept {}
	void Transcode::OnError(const std::string&) noexcept {}
	void Transcode::OnAborted() noexcept {}

	void Transcode::SetProgress(unsigned percent) noexcept {
		if (percent > 100)
			percent = 100;
		m_impl->m_progress.store(percent, std::memory_order_release);
		m_impl->m_hasProgress.store(true, std::memory_order_release);
		OnProgress(percent);
	}

	void Transcode::Worker() noexcept {
		*m_logger << Level::LowLevel << "Transcode::Worker enter" << std::endl;
		OnConfigure();
		if (m_impl->m_status.load(std::memory_order_acquire) == Status::Error) {
			OnError(Error().value_or("configure failed"));
			return;
		}
		if (m_impl->m_cancel.load(std::memory_order_acquire)) {
			m_impl->m_status.store(Status::Aborted, std::memory_order_release);
			OnAborted();
			return;
		}
		if (!m_impl->m_container || m_impl->m_path.empty()) {
			Fail("destination is not set");
			OnError(Error().value_or("destination is not set"));
			return;
		}

		std::vector<Slot> plan = m_impl->m_explicit;
		for (const auto& slot : plan) {
			if (!slot.copy && slot.codec == nullptr) {
				Fail("stream " + std::to_string(slot.in) + " has no Codec() or Copy()");
				OnError(Error().value_or("incomplete map"));
				return;
			}
		}

		std::set<int> used;
		for (const auto& slot : plan)
			used.insert(slot.in);
		for (int in : m_impl->m_ignore)
			used.insert(in);
		for (const auto& stream : m_file->Streams()) {
			if (used.contains(stream.Index()))
				continue;
			Slot implied;
			implied.in = stream.Index();
			implied.outKey = stream.Index();
			implied.kind = stream.Type();
			implied.copy = true;
			plan.push_back(std::move(implied));
		}

		std::sort(plan.begin(), plan.end(), [](const Slot& a, const Slot& b) {
			if (a.outKey != b.outKey)
				return a.outKey < b.outKey;
			return a.in < b.in;
		});

		const auto start = OnStart();
		if (start != Status::Running) {
			if (start == Status::Error) {
				if (!Failed())
					Fail("OnStart rejected the job");
				OnError(Error().value_or("OnStart rejected the job"));
			}
			else if (start == Status::Aborted) {
				m_impl->m_status.store(Status::Aborted, std::memory_order_release);
				OnAborted();
			}
			else {
				m_impl->m_status.store(Status::Stopped, std::memory_order_release);
			}
			return;
		}

		Demux demux;
		*m_file >> demux;
		if (demux.Failed()) {
			Fail(demux.Error().value_or("demux open failed"));
			OnError(Error().value_or("demux"));
			return;
		}

		Mux mux(*m_impl->m_container);
		mux >> m_impl->m_path;
		*m_file >> mux;
		if (mux.Failed()) {
			Fail(mux.Error().value_or("mux open failed"));
			OnError(Error().value_or("mux"));
			return;
		}

		struct EncodeLane {
			int in = -1;
			Type kind = Type::Video;
			std::unique_ptr<Decoder> decoder;
			std::unique_ptr<Encoder> encoder;
			PacketQueue* packets = nullptr;
			FrameQueue* frames = nullptr;
		};

		std::vector<std::unique_ptr<Copy>> copies;
		std::vector<EncodeLane> lanes;
		std::map<int, PacketQueue*> encodeByIn;
		std::set<int> copyIn;
		m_impl->m_muxQueue = std::make_unique<PacketQueue>(m_muxPacketCeiling);
		m_impl->m_copyQueue = std::make_unique<PacketQueue>(m_copyPacketCeiling);
		m_impl->m_encodeQueues.clear();
		m_impl->m_frameQueues.clear();

		int muxIndex = 0;
		for (auto& slot : plan) {
			if (slot.copy) {
				auto copy = std::make_unique<Copy>(muxIndex, slot.in);
				demux >> *copy;
				*copy >> mux;
				copyIn.insert(slot.in);
				copies.push_back(std::move(copy));
			}
			else {
				auto decoder = std::make_unique<Decoder>(slot.in);
				auto encoder = std::make_unique<Encoder>(muxIndex, *slot.codec);
				if (slot.implementation)
					encoder->Implementation(*slot.implementation);
				if (slot.crf)
					encoder->CRF(*slot.crf);
				if (slot.bitRate)
					encoder->BitRate(*slot.bitRate);
				if (slot.maxBitRate)
					encoder->MaxBitRate(*slot.maxBitRate);
				if (slot.preset)
					encoder->Preset(*slot.preset);
				if (slot.tune)
					encoder->Tune(*slot.tune);
				if (!slot.fineTune.empty())
					encoder->FineTune(slot.fineTune);
				if (slot.language)
					encoder->Language(*slot.language);
				if (slot.title)
					encoder->Title(*slot.title);
				demux >> *decoder;
				*encoder >> mux;
				const auto ceiling = slot.kind == Type::Video ? m_videoPacketCeiling : m_packetCeiling;
				m_impl->m_encodeQueues.push_back(std::make_unique<PacketQueue>(ceiling));
				EncodeLane lane;
				lane.in = slot.in;
				lane.kind = slot.kind;
				lane.decoder = std::move(decoder);
				lane.encoder = std::move(encoder);
				lane.packets = m_impl->m_encodeQueues.back().get();
				if (slot.kind == Type::Video) {
					m_impl->m_frameQueues.push_back(std::make_unique<FrameQueue>(m_videoFrameCeiling));
					lane.frames = m_impl->m_frameQueues.back().get();
				}
				encodeByIn[slot.in] = lane.packets;
				lanes.push_back(std::move(lane));
			}
			++muxIndex;
		}

		*m_logger << Level::Notice << "transcode running tracks=" << plan.size()
			<< " encode-lanes=" << lanes.size()
			<< " video-frames=" << m_videoFrameCeiling
			<< " video-packets=" << m_videoPacketCeiling
			<< " mux-packets=" << m_muxPacketCeiling
			<< " copy-packets=" << m_copyPacketCeiling << std::endl;
		const auto started = std::chrono::steady_clock::now();
		const auto duration = m_file->Duration();

		auto pushHeld = [this](std::deque<Packet>& held) {
			while (!held.empty()) {
				if (!m_impl->m_muxQueue->TryPush(held.front())) {
					m_impl->m_muxBlocked.fetch_add(1, std::memory_order_relaxed);
					return false;
				}
				held.pop_front();
				m_impl->NotifyIntake();
			}
			return true;
		};

		auto receiveAll = [this](Encoder& encoder, std::deque<Packet>& held) -> bool {
			for (;;) {
				Packet encoded;
				encoder >> encoded;
				if (encoder.Failed()) {
					Fail(encoder.Error().value_or("encoder failed"));
					return false;
				}
				if (encoded.StreamIndex() < 0)
					break;
				held.push_back(std::move(encoded));
				m_impl->m_recvPackets.fetch_add(1, std::memory_order_relaxed);
			}
			m_impl->m_heldSize.store(held.size(), std::memory_order_relaxed);
			return true;
		};

		auto drainEncoder = [this, &pushHeld, &receiveAll](Encoder& encoder, std::deque<Packet>& held) -> bool {
			if (!receiveAll(encoder, held))
				return false;
			(void)pushHeld(held);
			m_impl->m_heldSize.store(held.size(), std::memory_order_relaxed);
			return true;
		};

		auto drainDecoderToFrames = [this](Decoder& decoder, FrameQueue& frames) -> bool {
			for (;;) {
				Frame frame;
				decoder >> frame;
				if (decoder.Failed()) {
					Fail(decoder.Error().value_or("decoder failed"));
					return false;
				}
				if (frame.StreamIndex() < 0)
					break;
				frames.Push(std::move(frame), m_impl->m_cancel);
			}
			return true;
		};

		auto encodeOneFrame = [this, &drainEncoder](Encoder& encoder, Frame& frame,
			std::deque<Packet>& held) -> bool {
			frame >> encoder;
			if (encoder.Failed()) {
				Fail(encoder.Error().value_or("encoder failed"));
				return false;
			}
			return drainEncoder(encoder, held);
		};

		auto flushHeld = [this](std::deque<Packet>& held) {
			while (!held.empty() && !m_impl->m_cancel.load(std::memory_order_acquire)) {
				m_impl->m_muxQueue->Push(std::move(held.front()), m_impl->m_cancel);
				held.pop_front();
				m_impl->NotifyIntake();
			}
		};

		auto noteVideoProgress = [this, duration](const Packet& packet) {
			if (packet.StreamIndex() != 0)
				return;
			if (!packet.Pts() || !duration)
				return;
			const auto pts = packet.Pts()->Nanoseconds().count();
			const auto total = duration->Nanoseconds().count();
			if (total <= 0)
				return;
			unsigned next = static_cast<unsigned>((pts * 100) / total);
			if (next > 100)
				next = 100;
			const unsigned prev = m_impl->m_progress.load(std::memory_order_relaxed);
			if (!m_impl->m_hasProgress.load(std::memory_order_relaxed) || next > prev)
				SetProgress(next);
		};

		std::thread muxThread([this, &mux, &noteVideoProgress]() {
			while (!m_impl->m_cancel.load(std::memory_order_acquire)) {
				m_impl->WaitIfPaused();
				if (m_impl->m_cancel.load(std::memory_order_acquire))
					break;
				std::optional<Packet> packet = m_impl->m_muxQueue->TryPop();
				if (!packet)
					packet = m_impl->m_copyQueue->TryPop();
				if (!packet) {
					if (m_impl->m_muxQueue->Closed() && m_impl->m_copyQueue->Closed())
						break;
					m_impl->WaitForIntake();
					continue;
				}
				if (packet->StreamIndex() < 0)
					continue;
				noteVideoProgress(*packet);
				*packet >> mux;
				if (mux.Failed()) {
					Fail(mux.Error().value_or("mux write failed"));
					break;
				}
			}
		});

		std::vector<std::thread> workers;
		for (auto& lane : lanes) {
			if (lane.frames) {
				workers.emplace_back([this, &lane, &drainDecoderToFrames]() {
					while (!m_impl->m_cancel.load(std::memory_order_acquire)) {
						m_impl->WaitIfPaused();
						if (m_impl->m_cancel.load(std::memory_order_acquire))
							break;
						auto packet = lane.packets->Pop(m_impl->m_cancel);
						if (!packet)
							break;
						if (packet->StreamIndex() < 0)
							continue;
						*packet >> *lane.decoder;
						if (lane.decoder->Failed()) {
							Fail(lane.decoder->Error().value_or("decoder failed"));
							break;
						}
						if (!drainDecoderToFrames(*lane.decoder, *lane.frames))
							break;
					}
					if (!Failed() && !m_impl->m_cancel.load(std::memory_order_acquire)) {
						lane.decoder->Flush();
						if (lane.decoder->Failed())
							Fail(lane.decoder->Error().value_or("decoder flush failed"));
						else
							(void)drainDecoderToFrames(*lane.decoder, *lane.frames);
					}
					lane.frames->Close();
				});
				workers.emplace_back([this, &lane, &encodeOneFrame, &drainEncoder, &flushHeld, &pushHeld]() {
					std::deque<Packet> held;
					while (!m_impl->m_cancel.load(std::memory_order_acquire)) {
						m_impl->WaitIfPaused();
						if (m_impl->m_cancel.load(std::memory_order_acquire))
							break;
						if (!drainEncoder(*lane.encoder, held))
							break;
						if (auto frame = lane.frames->TryPop()) {
							if (frame->StreamIndex() < 0)
								continue;
							if (!encodeOneFrame(*lane.encoder, *frame, held))
								break;
							continue;
						}
						if (lane.frames->Closed())
							break;
						if (!held.empty() && m_impl->m_muxQueue->Full()) {
							m_impl->WaitForMuxSpace();
							continue;
						}
						auto frame = lane.frames->Pop(m_impl->m_cancel);
						if (!frame)
							break;
						if (frame->StreamIndex() < 0)
							continue;
						if (!encodeOneFrame(*lane.encoder, *frame, held))
							break;
					}
					if (!Failed() && !m_impl->m_cancel.load(std::memory_order_acquire)) {
						lane.encoder->Flush();
						if (lane.encoder->Failed())
							Fail(lane.encoder->Error().value_or("encoder flush failed"));
						else
							(void)drainEncoder(*lane.encoder, held);
						flushHeld(held);
					}
					else
						(void)pushHeld(held);
				});
			}
			else {
				workers.emplace_back([this, &lane, &drainEncoder, &flushHeld]() {
					std::deque<Packet> held;
					auto pump = [&]() -> bool {
						for (;;) {
							Frame frame;
							*lane.decoder >> frame;
							if (lane.decoder->Failed()) {
								Fail(lane.decoder->Error().value_or("decoder failed"));
								return false;
							}
							if (frame.StreamIndex() < 0)
								break;
							frame >> *lane.encoder;
							if (lane.encoder->Failed()) {
								Fail(lane.encoder->Error().value_or("encoder failed"));
								return false;
							}
							if (!drainEncoder(*lane.encoder, held))
								return false;
						}
						return true;
					};
					while (!m_impl->m_cancel.load(std::memory_order_acquire)) {
						m_impl->WaitIfPaused();
						if (m_impl->m_cancel.load(std::memory_order_acquire))
							break;
						auto packet = lane.packets->Pop(m_impl->m_cancel);
						if (!packet)
							break;
						if (packet->StreamIndex() < 0)
							continue;
						*packet >> *lane.decoder;
						if (lane.decoder->Failed()) {
							Fail(lane.decoder->Error().value_or("decoder failed"));
							break;
						}
						if (!pump())
							break;
					}
					if (!Failed() && !m_impl->m_cancel.load(std::memory_order_acquire)) {
						lane.decoder->Flush();
						if (!lane.decoder->Failed())
							(void)pump();
						lane.encoder->Flush();
						if (!lane.encoder->Failed())
							(void)drainEncoder(*lane.encoder, held);
						flushHeld(held);
					}
				});
			}
		}

		std::thread demuxThread([this, &demux, &encodeByIn, &copyIn]() {
			while (!m_impl->m_cancel.load(std::memory_order_acquire) && demux) {
				m_impl->WaitIfPaused();
				if (m_impl->m_cancel.load(std::memory_order_acquire))
					break;
				if (m_impl->m_copyQueue && m_impl->m_copyQueue->Full())
					m_impl->m_copyBlocked.fetch_add(1, std::memory_order_relaxed);
				Packet packet;
				demux >> packet;
				if (demux.Failed()) {
					Fail(demux.Error().value_or("demux read failed"));
					break;
				}
				if (demux.Eof() || packet.StreamIndex() < 0)
					break;
				const int in = packet.StreamIndex();
				if (m_impl->m_ignore.contains(in))
					continue;
				if (copyIn.contains(in)) {
					m_impl->m_copyQueue->Push(std::move(packet), m_impl->m_cancel);
					m_impl->NotifyIntake();
					continue;
				}
				auto found = encodeByIn.find(in);
				if (found == encodeByIn.end())
					continue;
				found->second->Push(std::move(packet), m_impl->m_cancel);
			}
			for (auto& queue : m_impl->m_encodeQueues)
				queue->Close();
			m_impl->m_copyQueue->Close();
			m_impl->NotifyIntake();
		});

		demuxThread.join();
		for (auto& thread : workers)
			thread.join();
		m_impl->m_muxQueue->Close();
		m_impl->m_copyQueue->Close();
		m_impl->NotifyIntake();
		m_impl->NotifySpace();
		muxThread.join();

		if (m_impl->m_cancel.load(std::memory_order_acquire)
			&& m_impl->m_status.load(std::memory_order_acquire) != Status::Error) {
			m_impl->m_status.store(Status::Aborted, std::memory_order_release);
			*m_logger << Level::Warning << "transcode aborted in "
				<< FormatElapsed(started) << std::endl;
			OnAborted();
			return;
		}

		if (!Failed()) {
			mux.Flush();
			if (mux.Failed())
				Fail(mux.Error().value_or("mux flush failed"));
		}

		if (Failed()) {
			OnError(Error().value_or("transcode failed"));
			return;
		}

		SetProgress(100);
		m_impl->m_status.store(Status::Done, std::memory_order_release);
		*m_logger << Level::Notice << "transcode done in " << FormatElapsed(started) << std::endl;
		OnDone();
	}
}
