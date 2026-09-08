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
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/filters/typedefs.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>
#include <StormByte/type_traits.hxx>

#include <array>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Demux;
	class Decoder;
	class Encoder;
	class Mux;

	Decoder& operator>>(Decoder& decoder, Frame& frame) noexcept;
	Demux& operator>>(Demux& demux, StormByte::Multimedia::Pipeline::Packet& packet) noexcept;
	StormByte::Multimedia::Pipeline::Packet& operator>>(StormByte::Multimedia::Pipeline::Packet& packet, Mux& mux) noexcept;
	Frame& operator>>(Frame& frame, Encoder& encoder) noexcept;
}

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter
 * @brief Facades, chain and reports for pipeline filter plugins.
 */
namespace StormByte::Multimedia::Pipeline::Filter {
	/**
	 * @class Chain
	 * @brief Ordered list of @ref FFmpeg nodes shared by pipeline stages.
	 *
	 * Instantiable. Not copyable, moveable. One list holds Process,
	 * Packet and Analytics nodes; each node ignores units and origins
	 * that are not its own.
	 *
	 * @par Ownership of the run
	 * Call, Eof and Reset are private. Friends are the four stages and
	 * the stream operators that push a unit through a stage. Flush and
	 * ReachedEof are stage members, so they already see Eof.
	 *
	 * @par Auto reset
	 * The first Call or Eof of a run calls Reset. Fail disarms at
	 * once. A clean end disarms when every origin that saw Call has
	 * seen Eof (and at least one Eof happened). The next Call/Eof
	 * starts a new run on the same instance. Stages stop pushing
	 * when Failed.
	 *
	 * @par Add
	 * Add is ignored while the run is armed. Add between runs so
	 * the next Reset sees the new node.
	 *
	 * The chain does not fail on its own. Failed / ErrorStr echo
	 * the node that called FFmpeg::Fail.
	 *
	 * Flush is private. Call runs it every FlushInterval units of
	 * that origin. Eof runs it once. A second Eof for the same
	 * origin is a no-op.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Chain {
		public:
			/**
			 * @name Lifetime
			 * @{
			 */

			/**
			 * @brief Empty list.
			 */
			Chain() noexcept = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Chain(const Chain&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other List to take.
			 */
			Chain(Chain&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Chain() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Chain& operator=(const Chain&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other List to take.
			 * @return *this.
			 */
			Chain& operator=(Chain&& other) noexcept = default;

			/** @} */

			/**
			 * @name Nodes
			 * @{
			 */

			/**
			 * @brief Appends a node. No-op if the run is armed, already Failed, or @p node is null.
			 * @param node Owned facade instance.
			 */
			void Add(std::unique_ptr<FFmpeg> node) noexcept;

			/**
			 * @brief Constructs and appends a node.
			 * @tparam FilterType Child of Process, Analytics or Packet.
			 * @param args Constructor arguments, forwarded.
			 * @return *this.
			 */
			template<typename FilterType, typename... Args>
			requires (
				!StormByte::Type::SameAs<FilterType, FFmpeg>
				&& !StormByte::Type::CopyConstructible<FilterType>
				&& StormByte::Type::MoveConstructible<FilterType>
				&& std::is_move_assignable_v<FilterType>
				&& (StormByte::Type::ConvertibleTo<FilterType*, Process*>
					|| StormByte::Type::ConvertibleTo<FilterType*, Analytics*>
					|| StormByte::Type::ConvertibleTo<FilterType*, Packet*>)
			)
			Chain& Add(Args&&... args) noexcept {
				Add(std::make_unique<FilterType>(std::forward<Args>(args)...));
				return *this;
			}

			/**
			 * @brief Number of nodes.
			 * @return Count.
			 */
			std::size_t Size() const noexcept;

			/** @} */

			/**
			 * @name Status
			 * @{
			 */

			/**
			 * @brief Whether a node has failed since the last Reset.
			 * @return true after a propagated FFmpeg::Fail.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text of the node that failed.
			 * @return The node's ErrorStr, unchanged.
			 *
			 * @pre Failed is true. Debug asserts; release returns empty.
			 */
			std::string ErrorStr() const noexcept;

			/** @} */

		private:
			friend class StormByte::Multimedia::Pipeline::Demux;
			friend class StormByte::Multimedia::Pipeline::Decoder;
			friend class StormByte::Multimedia::Pipeline::Encoder;
			friend class StormByte::Multimedia::Pipeline::Mux;
			friend Decoder& StormByte::Multimedia::Pipeline::operator>>(Decoder& decoder, Frame& frame) noexcept;
			friend Demux& StormByte::Multimedia::Pipeline::operator>>(Demux& demux, StormByte::Multimedia::Pipeline::Packet& packet) noexcept;
			friend StormByte::Multimedia::Pipeline::Packet& StormByte::Multimedia::Pipeline::operator>>(StormByte::Multimedia::Pipeline::Packet& packet, Mux& mux) noexcept;
			friend Frame& StormByte::Multimedia::Pipeline::operator>>(Frame& frame, Encoder& encoder) noexcept;

			/**
			 * @class Flags
			 * @brief Private @ref StormByte::Bitmask of @ref Origin.
			 */
			class STORMBYTE_MULTIMEDIA_PRIVATE Flags: public StormByte::Bitmask<Flags, Origin> {
				public:
					using Bitmask<Flags, Origin>::Bitmask;
			};

			static constexpr std::size_t FlushInterval = 64;	///< Calls per origin between private flushes

			std::vector<std::unique_ptr<FFmpeg>> m_nodes;					///< Nodes in declaration order
			bool m_failed = false;											///< Echo of a node Fail
			bool m_armed = false;											///< Current run has been Reset
			std::string m_reason;											///< Echo of a node ErrorStr
			Flags m_seen;													///< Origins that saw Call
			Flags m_eof;													///< Origins that saw Eof
			Flags m_eofFlushed;												///< Origins whose post-Eof Flush ran
			std::array<std::size_t, OriginCount> m_calls{};					///< Calls since last Flush

			/**
			 * @brief Stores a node ErrorStr and disarms.
			 * @param reason Text already formatted by the node.
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @brief Clears chain state and FFmpeg::Reset every node.
			 */
			void Reset() noexcept;

			/**
			 * @brief Calls every node with @p frame, then maybe Flush.
			 * @param frame Unit to mutate or measure.
			 * @param origin Stage that is calling.
			 */
			void Call(Pipeline::Frame& frame, Origin origin) noexcept;

			/**
			 * @brief Calls every node with @p packet, then maybe Flush.
			 * @param packet Unit to mutate.
			 * @param origin Stage that is calling.
			 */
			void Call(StormByte::Multimedia::Pipeline::Packet& packet, Origin origin) noexcept;

			/**
			 * @brief Ends @p origin once: nodes Eof, then Flush.
			 * @param origin Stage that ended.
			 */
			void Eof(Origin origin) noexcept;

			/**
			 * @brief Runs FFmpeg::Flush on every node for @p origin.
			 * @param origin Stage that is draining.
			 */
			void Flush(Origin origin) noexcept;

			/**
			 * @brief Arms the run if it is not Failed and not already armed.
			 */
			void EnsureArmed() noexcept;

			/**
			 * @brief Disarms after Fail, or when every seen origin has Eof.
			 */
			void MaybeDisarm() noexcept;

			/**
			 * @brief Array index for @p origin (`countr_zero` of the bit).
			 * @param origin Flag value.
			 * @return Zero-based index, or OriginCount if unknown.
			 */
			static std::size_t Index(Origin origin) noexcept;
	};
}
