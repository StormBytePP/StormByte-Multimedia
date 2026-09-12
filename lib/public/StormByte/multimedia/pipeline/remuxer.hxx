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

#include <StormByte/logger/log.hxx>
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstddef>
#include <memory>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Demuxer;
	class Muxer;
	class Remuxer;

	/**
	 * @brief Binds origin track remuxer.In() from @p demuxer onto @p remuxer.
	 * @param demuxer Origin demuxer.
	 * @param remuxer Destination remuxer.
	 * @return @p remuxer.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Remuxer& operator>>(Demuxer& demuxer, Remuxer& remuxer) noexcept;

	/**
	 * @class Remuxer
	 * @brief Forwards compressed packets of one origin track to the muxer.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Remuxer final: public Step {
		friend Remuxer& operator>>(Demuxer& demuxer, Remuxer& remuxer) noexcept;
		friend Remuxer& operator>>(Remuxer& remuxer, Muxer& muxer) noexcept;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Remuxer for origin stream @p in.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param in Origin stream index.
			 */
			explicit Remuxer(std::shared_ptr<StormByte::Logger::Log> log, int in) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source remuxer.
			 */
			Remuxer(const Remuxer& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Remuxer to take.
			 */
			Remuxer(Remuxer&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Remuxer() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source remuxer.
			 * @return *this.
			 */
			Remuxer& operator=(const Remuxer& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Remuxer to take.
			 * @return *this.
			 */
			Remuxer& operator=(Remuxer&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @brief Origin stream index.
			 * @return Index passed to the constructor.
			 */
			inline int In() const noexcept {
				return m_index;
			}

			/**
			 * @brief Ceiling of the remuxer input hopper.
			 * @return Max queued packets. Never 0.
			 */
			std::size_t InputCeiling() const noexcept override {
				return Ceiling;
			}

		private:
			using Step::Log;

			/**
			 * @brief Marks Ready. No codec.
			 */
			void Open() noexcept override;

			/**
			 * @brief Forwards one packet of In to m_out.
			 * @param item Incoming packet.
			 *
			 * The packet keeps the @ref Packet::Serial born at the demuxer.
			 * Missing lineage is a pipe error, not a drop.
			 */
			void Work(std::shared_ptr<Item> item) noexcept override;

			/**
			 * @brief No codec to flush.
			 */
			void Finish() noexcept override;

			static constexpr std::size_t Ceiling = 32;	///< Input hopper ceiling
			int m_index;								///< Origin stream index
	};
}
