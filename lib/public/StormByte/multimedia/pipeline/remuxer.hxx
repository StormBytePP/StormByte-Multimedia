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
	class Demuxer;
	class Muxer;
	class Remuxer;
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
	 * @brief Binds origin track @p remuxer.In() from @p demuxer onto @p remuxer.
	 * @param demuxer Open demuxer.
	 * @param remuxer Destination remuxer.
	 * @return @p remuxer.
	 *
	 * Propagates the Plan once. Does not reserve a muxer slot.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Remuxer& operator>>(Demuxer& demuxer, Remuxer& remuxer) noexcept;

	/**
	 * @brief Reserves a remux slot on @p muxer and binds hoppers.
	 * @param remuxer Origin remuxer.
	 * @param muxer Destination muxer.
	 * @return @p muxer.
	 *
	 * Slot order is the order of @c remuxer >> muxer / @c encoder >> muxer.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Muxer& operator>>(Remuxer& remuxer, Muxer& muxer) noexcept;

	/**
	 * @class Remuxer
	 * @brief Forwards compressed packets of one origin track to the muxer.
	 *
	 * A @ref Step, @c final. No decode and no encode. The factory for a
	 * remux track is @c demuxer >> remuxer >> muxer. @ref In is the origin
	 * stream index. The muxer slot is assigned when @c remuxer >> muxer runs.
	 *
	 * Packet / BSF filters sit in a @ref Route between Demuxer and Remuxer.
	 * Frame / Process filters are not valid on this stretch.
	 *
	 * Packets leaving @ref m_out keep @ref Item::Track == @ref In.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Remuxer final: public Step {
		friend Remuxer& operator>>(Demuxer& demuxer, Remuxer& remuxer) noexcept;
		friend Muxer& operator>>(Remuxer& remuxer, Muxer& muxer) noexcept;
		friend class Muxer;
		friend class Transcode;
		friend class Engine::Mux::Details::Container;
		friend class Engine::Transcode::Engine;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Remuxer for origin stream @p in.
			 * @param in Origin stream index.
			 *
			 * Starts the worker. The muxer slot is not chosen here.
			 */
			explicit Remuxer(int in) noexcept;

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

			int m_index;			///< Origin stream index
			Demuxer* m_demuxer;		///< Set by @c demuxer >> remuxer
	};
}
