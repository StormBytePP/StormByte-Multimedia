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

#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/pipeline/config/base.hxx>
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/transcode.hxx>
#include <StormByte/multimedia/type.hxx>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Transcode
 * @brief Job map and coordinator behind @ref StormByte::Multimedia::Pipeline::Transcode.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Transcode {
	/**
	 * @struct Slot
	 * @brief One explicit output track of the facade map.
	 *
	 * Intention lives in @ref Config. A null destination codec on the
	 * leaf is remux. Filters are Process / Packet only.
	 *
	 * @ingroup multimedia_pipeline
	 */
	struct STORMBYTE_MULTIMEDIA_PRIVATE Slot {
		int In = -1;																			///< Origin stream index
		int Out = -1;																			///< Mux slot
		StormByte::Multimedia::Type Kind = StormByte::Multimedia::Type::Unknown;				///< Media kind
		std::unique_ptr<StormByte::Multimedia::Pipeline::Config::Base> Config;					///< Intention leaf
		std::vector<std::shared_ptr<StormByte::Multimedia::Pipeline::Filter::FFmpeg>> Filters;	///< Per-track filters
		bool Settled = false;																	///< @ref Transcode::OnSettled already fired
	};

	/**
	 * @class Engine
	 * @brief Runtime of one @ref Transcode instance.
	 *
	 * Holds the fluent map, destination container, coordinator thread
	 * and lifecycle flags. Wires Demux / Route / Encoder / Mux when
	 * @ref Start runs. The facade only validates and fires hooks.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Engine {
		public:
			/**
			 * @brief Idle engine. Coordinator is not started.
			 */
			Engine() noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source engine.
			 */
			Engine(const Engine& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Engine to take.
			 */
			Engine(Engine&& other) noexcept = delete;

			/**
			 * @brief Destructor. Requests stop and joins the coordinator.
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
			 * @param other Engine to take.
			 * @return *this.
			 */
			Engine& operator=(Engine&& other) noexcept = delete;

			/**
			 * @brief Starts the coordinator if it is not already running.
			 * @param job Facade that owns this engine.
			 */
			void Start(StormByte::Multimedia::Pipeline::Transcode& job) noexcept;

			/**
			 * @brief Requests abort and wakes a paused coordinator.
			 */
			void RequestCancel() noexcept;

			/**
			 * @brief Joins the coordinator if it is joinable.
			 */
			void Join() noexcept;

			/**
			 * @brief Blocks while paused and not cancelled or failed.
			 */
			void WaitIfPaused() noexcept;

			mutable std::mutex Lock;											///< Guards Status text and Error
			std::mutex PauseMutex;												///< Mutex for @ref PauseCv
			std::condition_variable PauseCv;									///< Waiters of @ref WaitIfPaused
			std::atomic<StormByte::Multimedia::Pipeline::Status> Status {
				StormByte::Multimedia::Pipeline::Status::Stopped
			};																	///< Published lifecycle
			std::atomic<bool> Cancel { false };									///< Abort requested
			std::atomic<bool> Paused { false };									///< Pause latch
			std::atomic<bool> HasProgress { false };							///< Progress() has a value
			std::atomic<unsigned> Progress { 0 };								///< Last percent
			std::optional<std::string> Error;									///< Fail text
			const StormByte::Multimedia::Container* Container = nullptr;		///< Destination registry
			std::filesystem::path Path;											///< Output path
			std::vector<Slot> Mapped;											///< Fluent map; order is add order
			std::vector<std::shared_ptr<StormByte::Multimedia::Pipeline::Filter::FFmpeg>> Analytics;	///< Encode-lane analytics

		private:
			/**
			 * @brief Coordinator body. Builds the Plan and wires the job.
			 * @param job Facade.
			 * @param token Stop token of @ref m_worker.
			 */
			void Run(StormByte::Multimedia::Pipeline::Transcode& job, std::stop_token token) noexcept;

			std::jthread m_worker;												///< Coordinator
	};
}
