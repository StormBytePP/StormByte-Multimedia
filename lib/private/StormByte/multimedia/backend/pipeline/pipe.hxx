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
#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/visibility.h>

#include <condition_variable>
#include <cstddef>

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	/**
	 * @class Pipe
	 * @brief Input and output hoppers of one stage.
	 *
	 * Private. Step and Filter::FFmpeg compose one Pipe each.
	 * Not installed. @c operator>> between Pipes is redirect
	 * (Notify destination input, then Bind). It does not set
	 * Capacity: the call site uses the consumer's
	 * InputCeiling on @ref Capacity after Bind.
	 *
	 * Item flow is left to right, same as the public tube:
	 * @c pipe << item / @c item >> pipe write to Out (Push,
	 * blocks on the bound consumer Capacity). @c pipe >> item
	 * is Pop from In only; Wait stays on the Host.
	 *
	 * @ref CloneTo forks a copy of each write onto another Pipe
	 * (Route analytics look). No CloneTo means no clone.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Pipe {
		public:
			using Item = StormByte::Multimedia::Pipeline::Item;
			using ItemSink = StormByte::Buffer::Sink<Item::PointerType>;

			/**
			 * @brief Empty hoppers that Notify @p wake on Bind.
			 * @param wake Consumer CV of the owning stage.
			 */
			explicit Pipe(std::condition_variable& wake) noexcept;

			Pipe(const Pipe& other) = delete;
			Pipe(Pipe&& other) noexcept = delete;
			Pipe& operator=(const Pipe& other) = delete;
			Pipe& operator=(Pipe&& other) noexcept = delete;
			~Pipe() noexcept = default;

			/**
			 * @brief Input hopper.
			 * @return In sink.
			 */
			ItemSink& In() noexcept;

			/**
			 * @brief Input hopper.
			 * @return In sink.
			 */
			const ItemSink& In() const noexcept;

			/**
			 * @brief Output hopper.
			 * @return Out sink.
			 */
			ItemSink& Out() noexcept;

			/**
			 * @brief Output hopper.
			 * @return Out sink.
			 */
			const ItemSink& Out() const noexcept;

			/**
			 * @brief Ceiling of the input hopper for @p track.
			 * @param track Hopper key.
			 * @param n Max queued items. Caller skips this when 0.
			 *
			 * After Bind, Out and In are the same hopper: this is
			 * the only cap. There is no out ceiling.
			 */
			void Capacity(int track, std::size_t n) noexcept;

			/**
			 * @brief Registers @p wake on In (Launch and Bind).
			 */
			void Listen() noexcept;

			/**
			 * @brief Eof on In, Out and the optional clone hopper.
			 */
			void Close() noexcept;

			/**
			 * @brief Fork each write: clone onto @p dest In, original to Out.
			 * @param track Hopper key.
			 * @param dest Analytics (or look) consumer.
			 * @return @p dest.
			 *
			 * Notify dest In, Bind the clone hopper. @c operator<<
			 * then Clone s. Call before the producer Emits. No-op
			 * as a method if Route never calls it: writes do not clone.
			 */
			Pipe& CloneTo(int track, Pipe& dest) noexcept;

			/**
			 * @brief Drain Out until a consumer Binds.
			 */
			void Drain() noexcept;

			/**
			 * @brief Whether In has a unit or is EoF.
			 * @return @c In().Ready().
			 */
			bool Ready() const noexcept;

			/**
			 * @brief Whether In has seen Eof.
			 * @return @c In().EoF().
			 */
			bool InputEof() const noexcept;

			/**
			 * @class Lane
			 * @brief One-track redirect: @c from.To(track) >> dest.
			 */
			class STORMBYTE_MULTIMEDIA_PRIVATE Lane {
				public:
					/**
					 * @brief Notify dest In, Bind this track.
					 * @param dest Consumer pipe.
					 * @return @p dest.
					 */
					Pipe& operator>>(Pipe& dest) noexcept;

				private:
					friend class Pipe;
					Lane(Pipe& from, int track) noexcept;
					Pipe* m_from;
					int m_track;
			};

			/**
			 * @brief Redirect of one hopper key.
			 * @param track Hopper key.
			 * @return Lane for @c >> dest.
			 */
			Lane To(int track) noexcept;

			/**
			 * @brief Notify dest In, Bind every hopper.
			 * @param dest Consumer pipe.
			 * @return @p dest.
			 */
			Pipe& operator>>(Pipe& dest) noexcept;

			/**
			 * @brief Pop one unit from In into @p item. Does not Wait.
			 * @param item Destination pointer (may become empty).
			 * @return *this.
			 */
			Pipe& operator>>(Item::PointerType& item) noexcept;

			/**
			 * @brief Push @p item to Out. Blocks on dest Capacity.
			 * @param item Unit. Empty is a no-op.
			 * @return *this.
			 *
			 * Key is @c item->Track(). If @ref CloneTo ran, a Clone
			 * is Push ed to the clone hopper first (does not steal
			 * the original; that still blocks on dest Capacity).
			 */
			Pipe& operator<<(Item::PointerType item) noexcept;

			/**
			 * @brief Write: @p item flows into @p pipe Out.
			 * @param item Unit. Moved. Empty is a no-op.
			 * @param pipe Destination pipe.
			 * @return @p pipe.
			 */
			friend Pipe& operator>>(Item::PointerType& item, Pipe& pipe) noexcept;

			/**
			 * @brief Write: @p item flows into @p pipe Out.
			 * @param item Unit. Empty is a no-op.
			 * @param pipe Destination pipe.
			 * @return @p pipe.
			 */
			friend Pipe& operator>>(Item::PointerType&& item, Pipe& pipe) noexcept;

		private:
			std::condition_variable* m_wake;	///< Owner Wait CV
			ItemSink m_in;						///< Input buckets
			ItemSink m_out;						///< Output buckets
			ItemSink m_clone;					///< Analytics fork; unused until CloneTo
			bool m_fork;						///< CloneTo has bound m_clone
	};
}
