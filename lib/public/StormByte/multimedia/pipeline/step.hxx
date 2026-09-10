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

#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/pipeline/sink.hxx>
#include <StormByte/multimedia/visibility.h>

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Route;
	class Router;
	class Step;
	class Transcode;

	/**
	 * @brief Shares @p from output hoppers with @p to input.
	 * @param from Producer step.
	 * @param to Consumer step.
	 * @return @p to.
	 *
	 * Calls @c to.m_in.Wake(to.Wake()) and @c from.m_out.Bind(to.m_in).
	 * Does not create per-track buckets; @ref Route::Close /
	 * @ref Sink::Bind(int, Sink&) does that.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Step& operator>>(Step& from, Step& to) noexcept;

	/**
	 * @class Step
	 * @brief One threaded stage with an input @ref Sink and an output @ref Sink.
	 *
	 * Abstract. @ref Work is one consumed unit. Default @ref Work is a
	 * no-op so Demux need not override it. @ref Launch starts the
	 * worker. @ref Pump is the thread body. Default @ref Pump is the
	 * consumer loop. Demux overrides @ref Pump and reads the File.
	 * Mux uses the default loop and does not write to @ref m_out.
	 * Filters inherit Step as protected and expose @ref Launch through
	 * the filter facade.
	 *
	 * No @c In() / @c Out() getters: derived types and friends use
	 * @ref m_in / @ref m_out.
	 *
	 * No copy. Move is deleted. One condition variable. Bind of a
	 * track must run before a Push of that track unblocks.
	 * @ref Route, @ref Router, @ref Transcode and @c operator>> are
	 * friends.
	 *
	 * Demux input and Mux output stay at zero buckets until Bind.
	 * Decoder and Encoder are one track and one bucket.
	 *
	 * @ref Fail kills the job. Buckets have no Fail.
	 * Lifetime of a live step is a @c shared_ptr to the concrete type.
	 *
	 * Empty @ref Sink::Pop with @ref Sink::EoF is tube EoF.
	 * @ref Hopper::EoF may already be set while the bucket still has
	 * items; drain with Pop.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Step {
		friend class Route;
		friend class Router;
		friend class Transcode;
		friend Step& operator>>(Step& from, Step& to) noexcept;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Copy constructor.
			 * @param other Source step.
			 */
			Step(const Step& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Step to take.
			 */
			Step(Step&& other) noexcept = delete;

			/**
			 * @brief Destructor. Joins the worker if @ref Launch ran.
			 */
			virtual ~Step() noexcept;

			/**
			 * @brief Copy assignment.
			 * @param other Source step.
			 * @return *this.
			 */
			Step& operator=(const Step& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Step to take.
			 * @return *this.
			 */
			Step& operator=(Step&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @name Sinks
			 * @{
			 */

			/**
			 * @brief Consumer condition variable.
			 * @return The single CV of this step.
			 */
			std::condition_variable& Wake() noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Failure
			 * @{
			 */

			/**
			 * @brief Whether this step has failed.
			 * @return true after @ref Fail.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text.
			 * @return Message, or empty.
			 */
			const std::optional<std::string>& Error() const noexcept;

			/**
			 * @brief Marks a hard error and wakes the worker.
			 * @param reason Message. One failed step aborts the job.
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @}
			 */

		protected:
			/**
			 * @name Construction
			 * @{
			 */

			/**
			 * @brief Idle step. Sinks start at zero buckets.
			 */
			Step() noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Work
			 * @{
			 */

			/**
			 * @brief Prepare-once hook. Default no-op.
			 *
			 * Called from the default @ref Pump before the first Pop.
			 * Must be @c noexcept. On error call @ref Fail.
			 */
			virtual void Open() noexcept;

			/**
			 * @brief One unit taken from @ref m_in.
			 * @param item Never empty.
			 *
			 * Consumer body. Default no-op (Demux overrides @ref Pump
			 * instead). Must be @c noexcept. On error call @ref Fail.
			 */
			virtual void Work(std::shared_ptr<Item> item) noexcept;

			/**
			 * @brief Input is @ref Sink::EoF and the last Pop was empty.
			 *
			 * Called from the default @ref Pump before @ref m_out EoF.
			 * Default no-op. Must be @c noexcept.
			 */
			virtual void Finish() noexcept;

			/**
			 * @brief Body of the worker thread started by @ref Launch.
			 *
			 * Default: @ref Open, Pop / @ref Wait / @ref Work until
			 * EoF, @ref Finish, then @ref m_out EoF. Demux overrides
			 * this and reads the File. Must be @c noexcept.
			 */
			virtual void Pump() noexcept;

			/**
			 * @brief Sleeps on @ref Wake until @ref Sink::Ready or @ref Fail.
			 *
			 * Predicate: @ref Failed or @ref Sink::Ready. Ready is
			 * true when a hopper has an item or the input is EoF and
			 * empty. Does not pop. EoF may still have items; Ready
			 * then stays true until they are drained.
			 */
			void Wait() noexcept;

			/**
			 * @brief Starts the worker thread that runs @ref Pump.
			 *
			 * Idempotent no-op if the worker already runs.
			 */
			void Launch() noexcept;

			/**
			 * @}
			 */

			Sink m_in;									///< Input buckets
			Sink m_out;									///< Output buckets

		private:
			std::condition_variable m_wake;				///< Single consumer CV
			std::mutex m_wait;							///< Mutex for @ref m_wake
			std::jthread m_worker;						///< Owned worker
			std::atomic<bool> m_failed;					///< @ref Fail latched
			std::optional<std::string> m_error;			///< @ref Fail message
	};
}
