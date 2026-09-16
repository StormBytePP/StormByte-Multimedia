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

#include <StormByte/bitmask.hxx>
#include <StormByte/expected.hxx>
#include <StormByte/multimedia/pipeline/exception.hxx>
#include <StormByte/multimedia/visibility.h>
#include <StormByte/type_traits.hxx>

#include <cstdint>
#include <memory>

/**
 * @namespace StormByte::Buffer
 * @brief Hopper and Sink live in StormByte-Buffer. Forward only here.
 */
namespace StormByte::Buffer {
	template<Type::MoveConstructible T>
	class Sink;
}

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	using CheckResult = StormByte::Expected<void, PlanException>;	///< Outcome of @ref Plan::Check

	/**
	 * @enum Kind
	 * @brief Whether an @ref Item is a decoded frame or a compressed packet.
	 *
	 * Distinct from @ref StormByte::Multimedia::Type (Video / Audio / Subtitle).
	 * Values are bit flags. @c 0 is not a valid kind. Combine with @ref Kinds.
	 * @ref ToString understands a single flag only.
	 *
	 * @ingroup multimedia_pipeline
	 */
	enum class Kind: std::uint8_t {
		Packet = 1 << 0,	///< @ref StormByte::Multimedia::Pipeline::Packet
		Frame  = 1 << 1		///< @ref StormByte::Multimedia::Pipeline::Frame
	};

	/**
	 * @brief Converts a single @ref Kind flag to a string literal.
	 * @param kind Value to convert.
	 * @return `"Frame"`, `"Packet"`, or `"Invalid"` for a mask or zero.
	 */
	constexpr const char* ToString(Kind kind) noexcept {
		switch (kind) {
			case Kind::Frame:	return "Frame";
			case Kind::Packet:	return "Packet";
			default:			return "Invalid";
		}
	}

	/**
	 * @enum Producer
	 * @brief Named stage of the tube.
	 *
	 * Used as @ref Item origin and as @ref Step display name for logs.
	 * @c Filter covers @ref Filter::FFmpeg and its Process / Packet / Analytics leaves.
	 *
	 * @ingroup multimedia_pipeline
	 */
	enum class Producer: std::uint8_t {
		Demuxer,	///< @ref Demuxer
		Decoder,	///< @ref Decoder
		Remuxer,	///< @ref Remuxer
		Filter,		///< @ref Filter::FFmpeg
		Route,		///< @ref Route
		Router,		///< @ref Router
		Encoder,	///< @ref Encoder
		Muxer		///< @ref Muxer
	};

	/**
	 * @brief Converts a @ref Producer to a string literal.
	 * @param producer Value to convert.
	 * @return Null-terminated name, or `"Invalid"`.
	 */
	constexpr const char* ToString(Producer producer) noexcept {
		switch (producer) {
			case Producer::Demuxer:	return "Demuxer";
			case Producer::Decoder:	return "Decoder";
			case Producer::Remuxer:	return "Remuxer";
			case Producer::Filter:	return "Filter";
			case Producer::Route:	return "Route";
			case Producer::Router:	return "Router";
			case Producer::Encoder:	return "Encoder";
			case Producer::Muxer:	return "Muxer";
			default:				return "Invalid";
		}
	}

	/**
	 * @class Kinds
	 * @brief Bitmask of @ref Kind.
	 *
	 * Every @ref Step stores one mask as Receives and one as Produces.
	 * Empty means that side does not take or emit items (Demux receives
	 * nothing, Mux produces nothing).
	 *
	 * Tests use @ref StormByte::Bitmask::Has (all bits) and
	 * @ref StormByte::Bitmask::HasAny.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Kinds: public StormByte::Bitmask<Kinds, Kind> {
		public:
			using StormByte::Bitmask<Kinds, Kind>::Bitmask;
	};

	/**
	 * @enum State
	 * @brief Lifecycle of the stage thread. Values are mutually exclusive.
	 *
	 * This is the state of *this* stage, not of the job and not of
	 * the bound neighbor. A neighbor learns that this stage is dead
	 * because the shared hopper is Eof, not by reading this enum.
	 *
	 * Owned by the Pumper. A Worker has no State: it only Fail()s
	 * into the Host. Step::Status forwards to the Pumper.
	 *
	 * Created
	 *   Constructor finished. The object is usable: Plan can be
	 *   bound, operator>> can wire hoppers, Stop/Fail are legal.
	 *   Setup/Open has not returned. Pump must not take units
	 *   from the input hopper. Launch has usually already spawned
	 *   the thread; the thread is blocked inside Setup (waiting
	 *   for a Plan, an origin, a path, …).
	 *   This is the only state
	 *   in which Setup may run to completion.
	 *
	 * Ready
	 *   Setup returned. The backend (if any) can take work. Pump
	 *   may pop and call Process. There is no separate "running"
	 *   value: pumping is what a Ready thread does, not a new
	 *   phase. Fail or Stop may still fire from here.
	 *   When the input hopper reaches Eof and Pump returns without
	 *   Stop(), Launch moves Ready → Stopped. That is how a stage
	 *   ends in a live tube (Analytics after the last look, Muxer
	 *   after the last packet). Route::Idle and Transcoder wait
	 *   that Stopped before Reports / OnDone.
	 *
	 * Stopping
	 *   Stop() was called, or Halt() from a destructor. Hoppers
	 *   are Eof and waiters are notified. The thread has not
	 *   joined yet. Pump and Setup must return. This is not a
	 *   failure; the last Process({}) flush may still run.
	 *
	 * Stopped
	 *   The thread has left. Hoppers stay Eof. Reached from
	 *   Stopping after Halt/Stop, or from Ready after a natural
	 *   hopper Eof. The tube is not reused: there is no restart
	 *   back to Created.
	 *
	 * Failed
	 *   Fail() latched a reason. Hoppers are Eof. Terminal, like
	 *   Stopped, but Error() is set. May be entered from Created
	 *   (Setup never succeeded) or from Ready. Does not pass
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
}
