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

#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Demux;
	class Mux;
	class Remux;
	class Transcode;

	namespace Engine {
		namespace Mux {
			namespace Details {
				class Container;
			}
		}
		namespace Transcode {
			class Engine;
		}
	}

	/**
	 * @brief Binds origin track @p remux.In() from @p demux onto @p remux.
	 * @param demux Open demuxer.
	 * @param remux Destination remuxer.
	 * @return @p remux.
	 *
	 * Propagates the Plan once. Does not reserve a mux slot.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Remux& operator>>(Demux& demux, Remux& remux) noexcept;

	/**
	 * @brief Reserves a remux slot on @p mux and binds hoppers.
	 * @param remux Origin remuxer.
	 * @param mux Destination muxer.
	 * @return @p mux.
	 *
	 * Slot order is the order of @c remux >> mux / @c encoder >> mux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Mux& operator>>(Remux& remux, Mux& mux) noexcept;

	/**
	 * @class Remux
	 * @brief Forwards compressed packets of one origin track to the muxer.
	 *
	 * A @ref Step, @c final. No decode and no encode. The factory for a
	 * remux track is @c demux >> remux >> mux. @ref In is the origin
	 * stream index. The mux slot is assigned when @c remux >> mux runs.
	 *
	 * Packet / BSF filters sit in a @ref Route between Demux and Remux.
	 * Frame / Process filters are not valid on this stretch.
	 *
	 * Packets leaving @ref m_out keep @ref Item::Track == @ref In.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Remux final: public Step {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Remuxer for origin stream @p in.
			 * @param in Origin stream index.
			 *
			 * Starts the worker. The mux slot is not chosen here.
			 */
			explicit Remux(int in) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source remuxer.
			 */
			Remux(const Remux& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Remuxer to take.
			 */
			Remux(Remux&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Remux() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source remuxer.
			 * @return *this.
			 */
			Remux& operator=(const Remux& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Remuxer to take.
			 * @return *this.
			 */
			Remux& operator=(Remux&& other) noexcept = delete;

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

			friend Remux& operator>>(Demux& demux, Remux& remux) noexcept;
			friend Mux& operator>>(Remux& remux, Mux& mux) noexcept;
			friend class Mux;
			friend class Transcode;
			friend class Engine::Mux::Details::Container;
			friend class Engine::Transcode::Engine;

		protected:
			/**
			 * @brief Prepare-once. No codec open.
			 */
			void Open() noexcept override;

			/**
			 * @brief Forwards one packet of @ref In to @ref m_out.
			 * @param item Incoming packet.
			 */
			void Work(std::shared_ptr<Item> item) noexcept override;

			/**
			 * @brief No codec to flush.
			 */
			void Finish() noexcept override;

		private:
			/**
			 * @brief Marks a hard error.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			int m_index;		///< Origin stream index
			Demux* m_demux;		///< Set by @c demux >> remux
	};
}
