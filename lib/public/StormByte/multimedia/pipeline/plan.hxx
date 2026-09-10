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
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/track.hxx>
#include <StormByte/multimedia/pipeline/typedefs.hxx>
#include <StormByte/multimedia/visibility.h>

#include <filesystem>
#include <memory>
#include <utility>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Demux;

	/**
	 * @class Plan
	 * @brief Closed job intention: source File, destination container
	 *        and tracks that enter the tube.
	 *
	 * Move-only: the File is taken at construction. After the Plan
	 * is handed to the tube it is not needed again.
	 *
	 * Mux output order is the order of @ref add. `add` is not
	 * idempotent. A track omitted from @ref Tracks is dropped.
	 *
	 * @ref Check tests formation of this intention only. Success
	 * does not mean the tube will run.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Plan {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Takes @p source and binds the destination.
			 * @param source Probed origin (moved).
			 * @param container Registry destination container.
			 * @param destination Output path.
			 */
			Plan(StormByte::Multimedia::File&& source,
				const StormByte::Multimedia::Container& container,
				std::filesystem::path destination) noexcept;

			Plan(const Plan&) = delete;
			Plan& operator=(const Plan&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Plan to take.
			 */
			Plan(Plan&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			virtual ~Plan() noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Plan to take.
			 * @return *this.
			 */
			Plan& operator=(Plan&& other) noexcept = default;

			/**
			 * @}
			 */

			/**
			 * @brief Deep copy is not supported (`File` is move-only).
			 */
			std::unique_ptr<Plan> Clone() const = delete;

			/**
			 * @brief Move into a new pointer.
			 * @return Owning pointer to the moved @ref Plan.
			 */
			inline std::unique_ptr<Plan> Move() {
				return std::make_unique<Plan>(std::move(*this));
			}

			/**
			 * @name Job
			 * @{
			 */

			/**
			 * @brief Origin snapshot owned by this Plan.
			 * @return File.
			 */
			inline const StormByte::Multimedia::File& Source() const noexcept {
				return *m_source;
			}

			/**
			 * @brief Destination container (registry).
			 * @return Container.
			 */
			inline const StormByte::Multimedia::Container& Container() const noexcept {
				return *m_container;
			}

			/**
			 * @brief Output path.
			 * @return Path.
			 */
			inline const std::filesystem::path& Destination() const noexcept {
				return m_destination;
			}

			/**
			 * @brief Tube tracks. Index is mux slot.
			 * @return Track list.
			 */
			inline const class Tracks& Tracks() const noexcept {
				return m_tracks;
			}

			/**
			 * @brief Appends a clone of @p track. Not idempotent.
			 * @param track Track to copy.
			 */
			inline void add(const Track& track) {
				m_tracks.add(track);
			}

			/**
			 * @brief Appends @p track. Not idempotent. Order is mux order.
			 * @param track Track to take.
			 */
			inline void add(Track&& track) {
				m_tracks.add(std::move(track));
			}

			/**
			 * @}
			 */

			/**
			 * @brief Whether this intention is well formed.
			 *
			 * Success means the Plan is coherent as intention. It does
			 * not guarantee the tube will work.
			 *
			 * Fails when: the Plan was moved-from; @ref Destination is
			 * empty; @ref Tracks is empty; @ref Track::In is negative
			 * or not in @ref Source; @ref Track::Type is
			 * @ref StormByte::Multimedia::Type::Unknown or does not
			 * match the origin stream (or attachment slot);
			 * a destination @ref Config::Video / Audio / Subtitle
			 * codec has a different @ref StormByte::Multimedia::Codec::Type
			 * than the origin; an attachment MIME is empty or contains
			 * `*`. Duplicate @ref Track::In is allowed. Encode knobs on
			 * a Remux track (`Codec() == nullptr`) are ignored.
			 * Implementation pins, presets and container/codec pairing
			 * are not checked.
			 *
			 * @return Empty value, or @ref PlanException.
			 */
			virtual CheckResult Check() const;

		private:
			std::unique_ptr<StormByte::Multimedia::File> m_source;					///< Owned origin
			const StormByte::Multimedia::Container* m_container;					///< Registry destination
			std::filesystem::path m_destination;									///< Output path
			class Tracks m_tracks;													///< Tube tracks; index is mux slot
	};

	/**
	 * @brief Hands @p plan to @p demux. Not a Step bind.
	 * @param plan Intention (moved).
	 * @param demux Destination.
	 * @return @p demux.
	 *
	 * Stores the Plan on the Demux and wakes it. Does not
	 * call @ref Plan::Check and does not bind hoppers.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Demux& operator>>(Plan&& plan, Demux& demux) noexcept;
}
