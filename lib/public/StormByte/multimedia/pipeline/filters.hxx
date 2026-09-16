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

#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/filters/report.hxx>
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Route;

	/**
	 * @class Filters
	 * @brief Optional facade: Between stretches, Add filters, Close.
	 *
	 * A tube without filters does not need this. Ends are
	 * @c std::shared_ptr<Step>. operator>> still wires Plan and
	 * reserves (demuxer >> decoder, encoder >> muxer). Close
	 * wires each stretch (process chain, CloneTo, dest look).
	 *
	 * Global @ref Add is one analytics node shared by every
	 * matching Between (CloneTo, not N Launch). Per-stretch Add
	 * is @ref Handle::Add. Both are allowed; there is no dedup.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Filters {
		public:
			class Handle;

			Filters() noexcept;
			Filters(const Filters&) = delete;
			Filters(Filters&&) noexcept = delete;
			~Filters() noexcept;
			Filters& operator=(const Filters&) = delete;
			Filters& operator=(Filters&&) noexcept = delete;

			/**
			 * @brief Stretch from @p origin to @p destination.
			 * @param origin Decoder / Demuxer / Encoder.
			 * @param destination Encoder / Remuxer / Muxer.
			 * @return @ref Handle for per-stretch Add.
			 *
			 * Hopper key: Decoder::Index, else Remuxer::In / Encoder::Index
			 * of dest, else origin Encoder/Remuxer. Fail dest if unknown.
			 */
			Handle Between(std::shared_ptr<Step> origin,
				std::shared_ptr<Step> destination) noexcept;

			/**
			 * @brief Global analytics. One node, every matching stretch.
			 *
			 * Process / Packet leaves Fail: they go on @ref Handle::Add.
			 */
			Filters& Add(std::shared_ptr<Filter::FFmpeg> filter) noexcept;

			template<typename T, typename... Args>
			Filters& Add(Args&&... args) noexcept {
				return Add(std::shared_ptr<Filter::FFmpeg>(
					std::make_shared<T>(std::forward<Args>(args)...)));
			}

			void Close() noexcept;
			bool Idle() const noexcept;
			std::vector<std::pair<std::string, Filter::Report>> Reports() const noexcept;

			/**
			 * @class Handle
			 * @brief Per-stretch Add returned by @ref Between.
			 */
			class STORMBYTE_MULTIMEDIA_PUBLIC Handle {
				public:
					Handle& Add(std::shared_ptr<Filter::FFmpeg> filter) noexcept;

					template<typename T, typename... Args>
					Handle& Add(Args&&... args) noexcept {
						return Add(std::shared_ptr<Filter::FFmpeg>(
							std::make_shared<T>(std::forward<Args>(args)...)));
					}

				private:
					friend class Filters;
					Handle(Filters& owner, std::size_t index) noexcept;
					Filters* m_owner;
					std::size_t m_index;
			};

		private:
			struct Stretch {
				std::shared_ptr<Step> Origin;
				std::shared_ptr<Step> Destination;
				int Track = -1;
				std::unique_ptr<Route> Lane;
				std::optional<int> Scope;
			};

			struct Attached {
				std::shared_ptr<Filter::FFmpeg> Filter;
				std::optional<int> Track;
			};

			std::vector<Stretch> m_stretches;
			std::vector<Attached> m_globals;
			std::vector<Attached> m_reports;
	};
}
