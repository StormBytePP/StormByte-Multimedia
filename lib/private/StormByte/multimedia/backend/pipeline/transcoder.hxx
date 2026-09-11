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
#include <StormByte/multimedia/pipeline/transcoder.hxx>
#include <StormByte/multimedia/type.hxx>
#include <StormByte/multimedia/visibility.h>

#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	/**
	 * @class TranscoderSlot
	 * @brief One mapped output track of a Transcoder job.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE TranscoderSlot {
		public:
			int In = -1;								///< Origin stream index
			int Out = -1;								///< Mux destination order
			StormByte::Multimedia::Type Kind = StormByte::Multimedia::Type::Unknown;
			std::unique_ptr<StormByte::Multimedia::Pipeline::Config::Base> Config;
			std::vector<std::shared_ptr<StormByte::Multimedia::Pipeline::Filter::FFmpeg>> Filters;
			bool Settled = false;						///< OnSettled already fired
	};

	/**
	 * @class Transcoder
	 * @brief Runs one file-to-file job for the public Transcoder facade.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Transcoder {
		public:
			/**
			 * @brief Idle coordinator.
			 */
			Transcoder() noexcept;

			/**
			 * @brief Requests stop and joins the worker.
			 */
			~Transcoder() noexcept;

			Transcoder(const Transcoder& other) = delete;
			Transcoder& operator=(const Transcoder& other) = delete;
			Transcoder(Transcoder&& other) noexcept = delete;
			Transcoder& operator=(Transcoder&& other) noexcept = delete;

			/**
			 * @brief Starts the job thread if it is not already running.
			 * @param job Public facade.
			 */
			void Start(StormByte::Multimedia::Pipeline::Transcoder& job) noexcept;

			/**
			 * @brief Requests abort and wakes a paused worker.
			 */
			void RequestCancel() noexcept;

			/**
			 * @brief Joins the worker if it is joinable.
			 */
			void Join() noexcept;

			/**
			 * @brief Blocks while the job is paused.
			 */
			void WaitIfPaused() noexcept;

			mutable std::mutex Lock;					///< Status / Error
			std::mutex PauseMutex;						///< PauseCv
			std::condition_variable PauseCv;			///< Pause waiters
			std::atomic<StormByte::Multimedia::Pipeline::Status> Status {
				StormByte::Multimedia::Pipeline::Status::Stopped
			};
			std::atomic<bool> Cancel { false };
			std::atomic<bool> Paused { false };
			std::atomic<bool> HasProgress { false };
			std::atomic<unsigned> Progress { 0 };
			std::optional<std::string> Error;
			const StormByte::Multimedia::Container* Container = nullptr;
			std::filesystem::path Path;
			std::vector<TranscoderSlot> Mapped;
			std::vector<std::shared_ptr<StormByte::Multimedia::Pipeline::Filter::FFmpeg>> Analytics;

		private:
			/**
			 * @brief Builds the Plan, wires the tube and watches the job.
			 * @param job Public facade.
			 * @param token Stop token of the worker.
			 */
			void Run(StormByte::Multimedia::Pipeline::Transcoder& job, std::stop_token token) noexcept;

			std::jthread m_worker;						///< Coordinator thread
	};
}
