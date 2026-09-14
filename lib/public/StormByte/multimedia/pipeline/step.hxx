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

#include <StormByte/buffer/sink.hxx>
#include <StormByte/logger/log.hxx>
#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/pipeline/plan.hxx>
#include <StormByte/multimedia/pipeline/typedefs.hxx>
#include <StormByte/multimedia/visibility.h>

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

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
	 * Does not bind the analytics tap.
	 *
	 * @param from Producer step.
	 * @param to Consumer step.
	 * @return @p to.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC class Step& operator>>(Step& from, Step& to) noexcept;

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
	 *   When the input hopper reaches Eof and Pump returns without
	 *   Stop(), Launch moves Ready → Stopped. That is how a stage
	 *   ends in a live tube (Analytics after the last look, Muxer
	 *   after the last packet). Route::Idle and Transcoder wait
	 *   that Stopped before Reports / OnDone.
	 *
	 * Stopping
	 *   Stop() was called, or Halt() from a destructor. Hoppers
	 *   are Eof and waiters are notified. The worker has not
	 *   joined yet. Pump and Open must return. This is not a
	 *   failure; Finish may still run.
	 *
	 * Stopped
	 *   The worker has left. Hoppers stay Eof. Reached from
	 *   Stopping after Halt/Stop, or from Ready after a natural
	 *   hopper Eof. The tube is not reused: there is no restart
	 *   back to Created.
	 *
	 * Failed
	 *   Fail() latched a reason. Hoppers are Eof. Terminal, like
	 *   Stopped, but Error() is set. May be entered from Created
	 *   (Open never succeeded) or from Ready. Does not pass
	 *   through Stopping.
	 *
	 * Legal moves: Created→Ready, Created→Failed, Created→Stopping,
	 * Ready→Stopping, Ready→Failed, Ready→Stopped, Stopping→Stopped.
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
	 * @brief One threaded stage with an input @ref ItemSink, an output
	 *        @ref ItemSink and an analytics tap @ref ItemSink.
	 *
	 * Log lines use Logger component `STMM` and group @ref Label.
	 * Volume is a Logger throttle on that component/group, not a
	 * per-step counter.
	 *
	 * @ref Emit clones into @ref m_tap and then pushes the original
	 * to @ref m_out so a later @c Save cannot race the analytics look.
	 * @ref m_tap is constructed @c Drain: an unbound key drops the
	 * clone. @ref Route binds the tap before the origin emits.
	 *
	 * @ref Look is the encode-look hook. Default is a no-op.
	 * Encoder overrides it and deep-copies encoded packets into
	 * the look decoder sink. It is not a public Encoder API and
	 * it is not a copy of @ref m_out. @ref Route::TapEncode calls
	 * it through this Step hook.
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
			 * @brief Asks the worker to leave and closes all hoppers.
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
			 * @brief Marks Failed, closes all hoppers and wakes the worker.
			 *
			 * Bound neighbors unblock on hopper EoF. Does not join the
			 * worker; the worker may be the caller. Does not write a
			 * log line; the owner of the job logs Error.
			 *
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @}
			 */

		protected:
			using ItemSink = StormByte::Buffer::Sink<Item::PointerType>;

			/**
			 * @name Construction
			 * @{
			 */

			/**
			 * @brief Step in State::Created. Sinks start at zero buckets.
			 * @param log Shared logger. Prefer @c StormByte::Logger::ThreadedLog
			 *        when several workers write. A plain @c Log is accepted
			 *        for single-thread use. Empty pointer means no log.
			 * @param name Stage name used as the default @ref Label (Logger group).
			 * @param receives Kinds this step consumes.
			 * @param produces Kinds this step emits.
			 *
			 * @ref m_tap is @c Drain so an unbound analytics Push
			 * is dropped instead of waiting for @ref Route.
			 */
			Step(std::shared_ptr<StormByte::Logger::Log> log,
				enum Producer name,
				Kinds receives, Kinds produces) noexcept;

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
			virtual void Work(Item::PointerType item) noexcept;

			/**
			 * @brief Input is Sink EoF and the last Pop was empty.
			 */
			virtual void Finish() noexcept;

			/**
			 * @brief Encode-look side channel. Default no-op.
			 * @param sink Look decoder @c m_in.
			 *
			 * Encoder overrides and deep-copies each encoded packet
			 * into @p sink. Other leaves leave this empty. Not a copy
			 * of @ref m_out. @ref Route::TapEncode calls this hook;
			 * there is no public Encoder method for it.
			 */
			virtual void Look(ItemSink& sink) noexcept;

			/**
			 * @brief Clone into @ref m_tap, then push @p item to @ref m_out.
			 * @param item Unit to emit. Empty is a no-op.
			 *
			 * Always clones when @p item is set. If @ref m_tap has no
			 * hopper for that key, Drain drops the clone. Serial, Part
			 * and timing stay on the copy. Call this instead of pushing
			 * @ref m_out directly.
			 */
			void Emit(Item::PointerType item) noexcept;

			/**
			 * @brief Body of the worker after Open returns.
			 *
			 * Times each @ref Work. Leaves that override Pump (Demuxer)
			 * must call @ref RecordWork themselves if they want the same
			 * summary.
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
			 * @brief Adds one Work duration to the step summary.
			 * @param microseconds Wall time of that Work call.
			 */
			void RecordWork(std::int64_t microseconds) noexcept;

			/**
			 * @brief Duration of the last timed Work, or 0.
			 * @return Microseconds.
			 */
			std::int64_t LastWork() const noexcept;

			/**
			 * @brief Writes min/max Work time at Debug. No-op if none.
			 */
			void DumpWork() noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Logging
			 * @{
			 */

			/**
			 * @brief Logger group token for this step.
			 * @return @ref Producer name (`Encoder`) unless a leaf overrides
			 *         it (`Encoder(libx265)`). Called on every @ref Log.
			 */
			virtual std::string Label() const noexcept;

			/**
			 * @brief Writes one log line with component @c STMM and group @ref Label.
			 * @param level StormByte::Logger::Level of this line.
			 * @param message Already-formatted text (caller may use std::format).
			 *        Must not include the level name; Logger prints that.
			 *
			 * No-op when @ref m_log is empty. Virtual so a leaf can
			 * re-expose it to its backend (friendship is not inherited).
			 * Throttle belongs on the shared Logger, not here.
			 */
			virtual void Log(StormByte::Logger::Level level, std::string_view message) noexcept;

			/**
			 * @}
			 */

			std::shared_ptr<StormByte::Logger::Log> m_log;		///< Shared logger (ThreadedLog preferred)
			enum Producer m_name;								///< Default Label / Logger group
			ItemSink m_in;										///< Input buckets
			ItemSink m_out;										///< Output buckets (process path)
			ItemSink m_tap;										///< Analytics tap; Drain until Route binds
			Kinds m_receives;									///< Receives
			Kinds m_produces;									///< Produces

		private:
			/**
			 * @brief Eof on @ref m_in, @ref m_out and @ref m_tap.
			 */
			void CloseHoppers() noexcept;

			std::shared_ptr<class Plan> m_plan;					///< Current plan
			std::condition_variable m_wake;						///< Single consumer CV
			std::mutex m_wait;									///< Mutex for m_wake
			std::jthread m_worker;								///< Owned worker
			std::atomic<State> m_state;							///< Lifecycle
			std::optional<std::string> m_error;					///< Fail message
			std::uint64_t m_workN;								///< Timed Work calls
			std::int64_t m_workMin;								///< Fastest Work, us
			std::int64_t m_workMax;								///< Slowest Work, us
			std::int64_t m_lastWork;							///< Last Work, us
	};
}
