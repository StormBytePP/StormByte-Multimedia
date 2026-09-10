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
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/transcode.hxx>
#include <StormByte/multimedia/type.hxx>

#include <atomic>
#include <condition_variable>
#include <cstdint>
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
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Transcode {
	/**
	 * @brief One mapped or implied output track.
	 */
	struct Slot {
		int in = -1;														///< Source stream index
		int outKey = -1;													///< Destination order key
		StormByte::Multimedia::Type kind = StormByte::Multimedia::Type::Video;	///< Media kind
		bool copy = false;													///< Bitstream copy
		const StormByte::Multimedia::Codec* codec = nullptr;				///< Destination codec
		std::optional<std::string> implementation;							///< Pinned encoder name
		std::optional<int> crf;												///< CRF/CQ
		std::optional<std::int64_t> bitRate;								///< Target bitrate
		std::optional<std::int64_t> maxBitRate;								///< VBV ceiling
		std::optional<std::string> preset;									///< Preset
		std::optional<std::string> tune;									///< Tune
		std::map<std::string, std::string> fineTune;						///< Vendor leftovers
		std::optional<std::string> language;								///< Language override
		std::optional<std::string> title;									///< Title override
		std::optional<int> sampleFormat;									///< Settled AVSampleFormat
		std::optional<int> encoderChannels;									///< Settled encoder channels
		std::optional<int> frameSize;										///< Settled frame_size
		std::optional<int> settledRate;										///< Settled sample rate
		bool settled = false;												///< OnSettled already fired
		std::vector<std::shared_ptr<StormByte::Multimedia::Pipeline::Filter::FFmpeg>> filters;	///< Per-input-track filters
	};

	/**
	 * @class Engine
	 * @brief Map, lifecycle flags and the coordinator thread.
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
			 * @brief Copy constructor.
			 * @param other Source engine.
			 */
			Engine(const Engine& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source engine.
			 */
			Engine(Engine&& other) = delete;

			/**
			 * @brief Destructor. Cancels and joins the coordinator.
			 */
			~Engine() noexcept;

			/**
			 * @brief Copy assignment.
			 * @param other Source engine.
			 * @return *this.
			 */
			Engine& operator=(const Engine& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source engine.
			 * @return *this.
			 */
			Engine& operator=(Engine&& other) = delete;

			/**
			 * @brief Sets cancel and wakes the coordinator.
			 */
			void RequestCancel() noexcept;

			/**
			 * @brief Joins the coordinator thread if it is running.
			 */
			void Join() noexcept;

			/**
			 * @brief Blocks while the job is paused.
			 */
			void WaitIfPaused() noexcept;

			mutable std::mutex lock;											///< Status / error
			std::mutex pauseMutex;												///< Pause wait
			std::condition_variable pauseCv;									///< Pause waiters
			std::atomic<StormByte::Multimedia::Pipeline::Status> status {
				StormByte::Multimedia::Pipeline::Status::Stopped
			};																	///< Lifecycle
			std::atomic_bool cancel { false };									///< Abort requested
			std::atomic_bool paused { false };									///< Pause requested
			std::atomic<unsigned> progress { 0 };								///< Last percent
			std::atomic_bool hasProgress { false };								///< Progress() is valid
			std::optional<std::string> error;									///< Fail text
			const StormByte::Multimedia::Container* container = nullptr;		///< Destination
			std::filesystem::path path;											///< Output path
			std::vector<Slot> mapped;											///< Explicit tracks
			std::vector<std::shared_ptr<StormByte::Multimedia::Pipeline::Filter::FFmpeg>> analytics;	///< Job-level Analytics
			std::set<int> ignore;												///< Dropped source indexes
			std::thread worker;													///< Coordinator thread
	};
}
