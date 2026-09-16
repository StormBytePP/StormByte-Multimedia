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

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace StormByte::Multimedia::Backend::Pipeline {
	class Host;
	class Pumper;
	class Worker;
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
	 * Does not bind the analytics tap.
	 *
	 * @param from Producer step.
	 * @param to Consumer step.
	 * @return @p to.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC class Step& operator>>(Step& from, Step& to) noexcept;

	/**
	 * @class Step
	 * @brief One stage with an input @ref ItemSink, an output
	 *        @ref ItemSink and an analytics tap @ref ItemSink.
	 *
	 * Owns a Pumper (thread + @ref State) and exposes hoppers to
	 * that pumper through a private Host surface. Leaves Mount a
	 * concrete Pumper and Worker, then Launch.
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
			 * @brief Destructor. Joins the pumper if Launch ran.
			 *
			 * Most-derived Step members are already gone. Those
			 * leaves Halt first via @ref Join so a backend the
			 * worker still uses survives until the thread has left.
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
			 * @return @ref State of this step only.
			 */
			State Status() const noexcept;

			/**
			 * @brief Whether this step can take work.
			 *
			 * Default: Status is Ready (Setup finished without Fail or Stop).
			 * Muxer also requires @ref Muxer::Armed so header write and
			 * Pump do not start while @c operator>> is still reserving.
			 *
			 * @return true when the stage is open for work.
			 */
			virtual bool Ready() const noexcept;

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
			 * @brief Deep-copies @p item through @ref Item::Clone.
			 * @param item Unit to clone.
			 * @return Owning pointer, or empty if @p item cannot clone.
			 *
			 * Packet/Frame copy constructors stay private. Looks and
			 * the analytics tap must go through this helper (or
			 * @ref Emit) so a leaf Step does not need to be a friend
			 * of the unit types.
			 */
			Item::PointerType CloneItem(const Item& item) const noexcept;

			/**
			 * @brief Sleeps on Wake until hopper Ready, Failed or Stopping.
			 */
			void Wait() noexcept;

			/**
			 * @brief Starts the pumper: Setup, Ready, Pump.
			 *
			 * Idempotent. No-op without a mounted pumper, or if the
			 * stage has already Failed or Stopped.
			 */
			void Launch() noexcept;

			/**
			 * @brief Stop and join the pumper.
			 *
			 * Safe to call more than once. Step leaves do not call
			 * this from their destructor: @ref Join is their last
			 * member and Halt s before other members die. Filter
			 * plugins never call Halt; @ref Route joins them while
			 * the leaf is still complete.
			 */
			void Halt() noexcept;

			/**
			 * @class Join
			 * @brief Last data member of every most-derived Step.
			 *
			 * Destructor Halt s this Step. Declared last so backends
			 * and hoppers of the leaf are still alive while the
			 * worker leaves. Not a plugin API: Filter::FFmpeg is
			 * not a Step.
			 */
			class Join final {
				public:
					explicit Join(Step& step) noexcept: m_step(step) {}
					Join(const Join&) = delete;
					Join(Join&&) noexcept = delete;
					~Join() noexcept {
						m_step.Halt();
					}
					Join& operator=(const Join&) = delete;
					Join& operator=(Join&&) noexcept = delete;

				private:
					Step& m_step;
			};

			/**
			 * @brief Worker must return (Stop, Halt or Fail).
			 * @return true if Status is Stopping, Stopped or Failed.
			 */
			bool Stopping() const noexcept;

			/**
			 * @brief Adds one Process duration to the step summary.
			 * @param microseconds Wall time of that Process call.
			 */
			void RecordWork(std::int64_t microseconds) noexcept;

			/**
			 * @brief Duration of the last timed Process, or 0.
			 * @return Microseconds.
			 */
			std::int64_t LastWork() const noexcept;

			/**
			 * @brief Writes min/max Process time at Debug. No-op if none.
			 */
			void DumpWork() noexcept;

			/**
			 * @brief Owner surface for the Pumper and Worker.
			 * @return Host implemented by this Step.
			 */
			Backend::Pipeline::Host& Face() noexcept;

			/**
			 * @brief Takes ownership of @p pumper and binds @p worker.
			 * @param pumper Source, Through or Sink. Must not be empty.
			 * @param worker Stage body. Must not be empty.
			 *
			 * No-op if a pumper is already mounted. Does not Launch.
			 */
			void Mount(std::unique_ptr<Backend::Pipeline::Pumper> pumper,
				std::unique_ptr<Backend::Pipeline::Worker> worker) noexcept;

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
			ItemSink m_out;									///< Output buckets (process path)
			ItemSink m_tap;									///< Analytics tap; Drain until Route binds
			ItemSink m_lookOut;								///< Packet-look producer for Demuxer remux stretch
			Kinds m_receives;									///< Receives
			Kinds m_produces;									///< Produces

		private:
			class Surface;

			/**
			 * @brief Eof on @ref m_in, @ref m_out and @ref m_tap.
			 */
			void CloseHoppers() noexcept;

			std::unique_ptr<Surface> m_surface;					///< Host for Pumper and Worker
			std::unique_ptr<Backend::Pipeline::Pumper> m_pumper;	///< Thread and State
			std::shared_ptr<class Plan> m_plan;					///< Current plan
			std::condition_variable m_wake;						///< Single consumer CV
			std::mutex m_wait;									///< Mutex for m_wake
			std::optional<std::string> m_error;					///< Fail message
			bool m_exhausted;									///< Source Ended()
			std::uint64_t m_workN;								///< Timed Process calls
			std::int64_t m_workMin;								///< Fastest Process, us
			std::int64_t m_workMax;								///< Slowest Process, us
			std::int64_t m_lastWork;							///< Last Process, us
	};
}
