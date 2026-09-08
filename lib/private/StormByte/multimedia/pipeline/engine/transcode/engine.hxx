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

#include <StormByte/multimedia/codec.hxx>
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/transcode.hxx>
#include <StormByte/multimedia/type.hxx>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Transcode
 * @brief Private runtime behind Pipeline::Transcode.
 */
namespace StormByte::Multimedia::Pipeline::Engine::Transcode {
	/**
	 * @brief One mapped or implied output track.
	 */
	struct Slot {
		int in = -1;													///< Source stream index
		int outKey = -1;												///< Destination order key
		StormByte::Multimedia::Type kind = StormByte::Multimedia::Type::Video;	///< Media kind
		bool copy = false;												///< Bitstream copy
		const StormByte::Multimedia::Codec* codec = nullptr;			///< Destination codec
		std::optional<std::string> implementation;						///< Pinned encoder name
		std::optional<int> crf;											///< CRF/CQ
		std::optional<std::int64_t> bitRate;							///< Target bitrate
		std::optional<std::int64_t> maxBitRate;							///< VBV ceiling
		std::optional<std::string> preset;								///< Preset
		std::optional<std::string> tune;								///< Tune
		std::map<std::string, std::string> fineTune;					///< Vendor leftovers
		std::optional<std::string> language;							///< Language override
		std::optional<std::string> title;								///< Title override
		std::optional<int> sampleFormat;								///< Settled AVSampleFormat
		std::optional<int> encoderChannels;								///< Settled encoder channels
		std::optional<int> frameSize;									///< Settled frame_size
		std::optional<int> settledRate;									///< Settled sample rate
		bool settled = false;											///< OnSettled already fired
	};

	/**
	 * @brief Bounded MPMC-ish queue used by the job workers.
	 * @tparam Item Packet or Frame.
	 */
	template<typename Item>
	class BoundQueue {
		public:
			/**
			 * @brief Constructs a queue with at least one slot.
			 * @param ceiling Maximum stored items.
			 */
			explicit BoundQueue(std::size_t ceiling) noexcept;

			/**
			 * @brief Blocks until there is room, then stores @p item.
			 * @param item Value to enqueue.
			 * @param cancel Abort flag.
			 */
			void Push(Item item, const std::atomic_bool& cancel) noexcept;

			/**
			 * @brief Non-blocking push.
			 * @param item Value to enqueue.
			 * @return false if full or closed.
			 */
			bool TryPush(Item item) noexcept;

			/**
			 * @brief Blocks until an item is available or the queue closes.
			 * @param cancel Abort flag.
			 * @return Item, or empty on cancel/close.
			 */
			std::optional<Item> Pop(const std::atomic_bool& cancel) noexcept;

			/**
			 * @brief Non-blocking pop.
			 * @return Item, or empty if the queue is empty.
			 */
			std::optional<Item> TryPop() noexcept;

			/**
			 * @brief Whether the queue holds no items.
			 * @return true if empty.
			 */
			bool Empty() const noexcept;

			/**
			 * @brief Whether the queue is at ceiling.
			 * @return true if full.
			 */
			bool Full() const noexcept;

			/**
			 * @brief Closed and drained.
			 * @return true if producers finished and the queue is empty.
			 */
			bool Closed() const noexcept;

			/**
			 * @brief Marks the queue closed and wakes waiters.
			 */
			void Close() noexcept;

			/**
			 * @brief Wakes every waiter without closing.
			 */
			void Wake() noexcept;

		private:
			std::size_t m_ceiling;										///< Maximum stored items
			std::deque<Item> m_queue;									///< Stored items
			mutable std::mutex m_mutex;									///< Queue lock
			std::condition_variable m_cv;								///< Waiters
			bool m_closed = false;										///< Producer finished
	};

	using PacketQueue = BoundQueue<StormByte::Multimedia::Pipeline::Packet>;
	using FrameQueue = BoundQueue<StormByte::Multimedia::Pipeline::Frame>;

	/**
	 * @class Engine
	 * @brief Queues, worker thread and lifecycle flags for one Transcode job.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class Engine {
		public:
			/**
			 * @brief Constructs an idle engine.
			 */
			Engine() noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Engine(const Engine&) = delete;

			/**
			 * @brief Move constructor (deleted).
			 */
			Engine(Engine&&) = delete;

			/**
			 * @brief Destructor. Cancels and joins the worker.
			 */
			~Engine() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Engine& operator=(const Engine&) = delete;

			/**
			 * @brief Move assignment (deleted).
			 * @return *this.
			 */
			Engine& operator=(Engine&&) = delete;

			/**
			 * @brief Wakes the mux intake waiter.
			 */
			void NotifyIntake() noexcept;

			/**
			 * @brief Blocks until a mux/copy packet arrives or both queues close.
			 */
			void WaitForIntake() noexcept;

			/**
			 * @brief Sets cancel and wakes every waiter.
			 */
			void RequestCancel() noexcept;

			/**
			 * @brief Joins the job thread if it is running.
			 */
			void Join() noexcept;

			/**
			 * @brief Blocks while the job is paused.
			 */
			void WaitIfPaused() noexcept;

			/**
			 * @brief Runs the pipeline for @p owner. Called from the job thread.
			 * @param owner Public Transcode.
			 */
			void Run(StormByte::Multimedia::Pipeline::Transcode& owner) noexcept;

			mutable std::mutex lock;										///< Status / error
			std::mutex pauseMutex;											///< Pause wait
			std::condition_variable pauseCv;								///< Pause waiters
			std::mutex intakeMutex;											///< Mux intake wait
			std::condition_variable intakeCv;								///< Mux intake waiters
			std::atomic<StormByte::Multimedia::Pipeline::Status> status {
				StormByte::Multimedia::Pipeline::Status::Stopped
			};																///< Lifecycle
			std::atomic_bool cancel { false };								///< Abort requested
			std::atomic_bool paused { false };								///< Pause requested
			std::atomic<unsigned> progress { 0 };							///< Last percent
			std::atomic_bool hasProgress { false };							///< Progress() is valid
			std::optional<std::string> error;								///< Fail text
			const StormByte::Multimedia::Container* container = nullptr;	///< Destination
			std::filesystem::path path;										///< Output path
			std::vector<Slot> mapped;										///< Explicit tracks
			std::set<int> ignore;											///< Dropped source indexes
			std::thread worker;												///< Job thread
			std::unique_ptr<PacketQueue> muxQueue;							///< Recode packets
			std::unique_ptr<PacketQueue> copyQueue;							///< Copy packets
			std::vector<std::unique_ptr<PacketQueue>> encodeQueues;			///< Per-lane compressed packets
			std::vector<std::unique_ptr<FrameQueue>> frameQueues;			///< Video frame queues
	};
}
