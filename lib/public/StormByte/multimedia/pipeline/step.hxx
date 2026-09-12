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
#include <StormByte/multimedia/pipeline/plan.hxx>
#include <StormByte/multimedia/pipeline/typedefs.hxx>
#include <StormByte/multimedia/visibility.h>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

/**
 * @namespace StormByte::Multimedia::Buffer
 * @brief Private tube queues.
 */
namespace StormByte::Multimedia::Buffer {
	class Sink;
}

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Decoder;
	class Demuxer;
	class Encoder;
	class Muxer;
	class Remuxer;
	class Route;
	class Router;
	class Step;

	/**
	 * @brief Shares Plan and binds every output hopper of @p from onto @p to.
	 *
	 * Demuxer fan-out is not this operator. A demuxer binds one origin
	 * index at a time from the leaf operator>>.
	 *
	 * @param from Producer step.
	 * @param to Consumer step.
	 * @return @p to.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Step& operator>>(Step& from, Step& to) noexcept;

	/**
	 * @enum State
	 * @brief Lifecycle of one Step. Values are mutually exclusive.
	 *
	 * This is the state of *this* stage, not of the job and not of
	 * the bound neighbor. A neighbor learns that this stage is dead
	 * because the shared hopper is Eof, not by reading this enum.
	 *
	 * Created
	 *   Constructor finished. The object is usable: Plan can be
	 *   bound, operator>> can wire hoppers, Stop/Fail are legal.
	 *   Open has not returned. Pump must not take units from m_in.
	 *   Launch has usually already spawned the worker; the worker
	 *   is blocked inside Open (waiting for a Plan, an origin,
	 *   a path, …). This is the only state in which Open may run
	 *   to completion.
	 *
	 * Ready
	 *   Step::Open returned. The backend (if any) can take work.
	 *   Pump may pop m_in and call Work. There is no separate
	 *   "running" value: pumping is what a Ready worker does, not
	 *   a new phase. Fail or Stop may still fire from here.
	 *
	 * Stopping
	 *   Stop() was called, or Halt() from a destructor. Hoppers
	 *   are Eof and waiters are notified. The worker has not
	 *   joined yet. Pump and Open must return. This is not a
	 *   failure; Finish may still run.
	 *
	 * Stopped
	 *   The worker has left. Hoppers stay Eof. The tube is not
	 *   reused: there is no restart back to Created.
	 *
	 * Failed
	 *   Fail() latched a reason. Hoppers are Eof. Terminal, like
	 *   Stopped, but Error() is set. May be entered from Created
	 *   (Open never succeeded) or from Ready. Does not pass
	 *   through Stopping.
	 *
	 * Legal moves: Created→Ready, Created→Failed, Created→Stopping,
	 * Ready→Stopping, Ready→Failed, Stopping→Stopped.
	 * Failed and Stopped do not leave.
	 *
	 * Source EOF is not a State. Demuxer keeps m_eof next to this.
	 *
	 * @ingroup multimedia_pipeline
	 */
	enum class State: std::uint8_t {
		Created,
		Ready,
		Stopping,
		Stopped,
		Failed
	};

	/**
	 * @class Step
	 * @brief One threaded stage with an input Sink and an output Sink.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Step {
		friend class Demuxer;
		friend class Muxer;
		friend class Remuxer;
		friend class Route;
		friend class Router;
		friend Decoder& operator>>(Demuxer& demuxer, Decoder& decoder) noexcept;
		friend Demuxer& operator>>(class Plan&& plan, Demuxer& demuxer) noexcept;
		friend Encoder& operator>>(Encoder& encoder, Muxer& muxer) noexcept;
		friend Muxer& operator>>(Demuxer& demuxer, Muxer& muxer) noexcept;
		friend Remuxer& operator>>(Demuxer& demuxer, Remuxer& remuxer) noexcept;
		friend Remuxer& operator>>(Remuxer& remuxer, Muxer& muxer) noexcept;
		friend Step& operator>>(Step& from, Step& to) noexcept;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			Step(const Step& other) = delete;
			Step(Step&& other) noexcept = delete;

			/**
			 * @brief Destructor. Signals stop and joins the worker if Launch ran.
			 */
			virtual ~Step() noexcept;

			Step& operator=(const Step& other) = delete;
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
			 * @name Plan
			 * @{
			 */

			/**
			 * @brief Bound job intention, if any.
			 * @return Shared Plan, or empty.
			 */
			inline const std::shared_ptr<class Plan>& Plan() const noexcept {
				return m_plan;
			}

			/**
			 * @}
			 */

			/**
			 * @name Flow
			 * @{
			 */

			/**
			 * @brief Kinds this step consumes.
			 * @return Mask set at construction.
			 */
			inline const Kinds& Receives() const noexcept {
				return m_receives;
			}

			/**
			 * @brief Kinds this step emits.
			 * @return Mask set at construction.
			 */
			inline const Kinds& Produces() const noexcept {
				return m_produces;
			}

			/**
			 * @brief Current lifecycle value.
			 * @return State of this step only.
			 */
			State Status() const noexcept;

			/**
			 * @brief Whether Open has finished without Fail or Stop.
			 * @return true iff Status is Ready.
			 */
			bool Ready() const noexcept;

			/**
			 * @brief Asks the worker to leave and closes both hoppers.
			 *
			 * Neighbors unblock on hopper EoF. Does not join; the
			 * worker may be the caller. Idempotent. A mounted tube
			 * is not restarted after Stop.
			 */
			void Stop() noexcept;

			/**
			 * @brief Ceiling of this step's input hopper.
			 * @return Max queued items. 0 means unbounded.
			 *
			 * Leaves override this and return their private Ceiling.
			 * Bind sites call it after creating the destination bucket.
			 */
			virtual std::size_t InputCeiling() const noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Failure
			 * @{
			 */

			/**
			 * @brief Whether this step has failed.
			 * @return true iff Status is Failed.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text.
			 * @return Message, or empty.
			 */
			const std::optional<std::string>& Error() const noexcept;

			/**
			 * @brief Marks Failed, closes both hoppers and wakes the worker.
			 *
			 * Bound neighbors unblock on hopper EoF. Does not join the
			 * worker; the worker may be the caller.
			 *
			 * @param reason Message.
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
			 * @brief Step in State::Created. Sinks start at zero buckets.
			 * @param receives Kinds this step consumes.
			 * @param produces Kinds this step emits.
			 */
			Step(Kinds receives, Kinds produces) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Work
			 * @{
			 */

			/**
			 * @brief Prepare-once hook. Moves Created → Ready and wakes waiters.
			 *
			 * Leaves that override Open must call Step::Open at the end
			 * after their backend is usable. Must be noexcept. On error
			 * call Fail and return without Step::Open.
			 */
			virtual void Open() noexcept;

			/**
			 * @brief One unit taken from m_in.
			 * @param item Never empty.
			 */
			virtual void Work(std::shared_ptr<Item> item) noexcept;

			/**
			 * @brief Input is Sink EoF and the last Pop was empty.
			 */
			virtual void Finish() noexcept;

			/**
			 * @brief Body of the worker after Open returns.
			 */
			virtual void Pump() noexcept;

			/**
			 * @brief Sleeps on Wake until hopper Ready, Failed or Stopping.
			 */
			void Wait() noexcept;

			/**
			 * @brief Starts the worker: Open, then Pump if still Ready.
			 */
			void Launch() noexcept;

			/**
			 * @brief Stop and join the worker.
			 *
			 * Safe to call more than once. Leaves call this from their
			 * destructor before releasing a backend the worker still uses.
			 */
			void Halt() noexcept;

			/**
			 * @brief Worker must return (Stop, Halt or Fail).
			 * @return true if Status is Stopping, Stopped or Failed.
			 */
			bool Stopping() const noexcept;

			/**
			 * @}
			 */

			std::unique_ptr<Buffer::Sink> m_in;			///< Input buckets
			std::unique_ptr<Buffer::Sink> m_out;		///< Output buckets
			Kinds m_receives;							///< Receives
			Kinds m_produces;							///< Produces

		private:
			std::shared_ptr<class Plan> m_plan;			///< Current plan
			std::condition_variable m_wake;				///< Single consumer CV
			std::mutex m_wait;							///< Mutex for m_wake
			std::jthread m_worker;						///< Owned worker
			std::atomic<State> m_state;					///< Lifecycle
			std::optional<std::string> m_error;			///< Fail message
	};
}
