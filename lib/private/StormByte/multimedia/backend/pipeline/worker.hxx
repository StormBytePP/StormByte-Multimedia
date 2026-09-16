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

#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/pipeline/typedefs.hxx>
#include <StormByte/multimedia/visibility.h>

#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	/**
	 * @class Worker
	 * @brief Stage body without a thread.
	 *
	 * A Pumper calls @ref Setup once (idempotent) and
	 * @ref Process each cycle. Concretes: DemuxWorker,
	 * DecodeWorker, EncodeWorker, RemuxWorker, MuxWorker,
	 * FilterWorker.
	 *
	 * Source pumpers pass an empty pointer; through and sink
	 * pass a popped unit. An empty return means do not push.
	 * Each concrete calls its own @ref Flush when it sees EoF.
	 * Hoppers belong to the owner.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Worker {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			Worker(const Worker&) = delete;
			Worker(Worker&&) noexcept = delete;
			virtual ~Worker() noexcept = default;
			Worker& operator=(const Worker&) = delete;
			Worker& operator=(Worker&&) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @brief Lifecycle value of this worker.
			 * @return Current @ref State.
			 */
			Multimedia::Pipeline::State Status() const noexcept;

			/**
			 * @brief One-shot backend bring-up.
			 *
			 * Must be noexcept. Idempotent: Created performs the
			 * work; any other State is a no-op. On error call
			 * @ref Fail and return. The Pumper sets Ready only if
			 * Status is still Created after return.
			 */
			virtual void Setup() noexcept = 0;

			/**
			 * @brief One cycle of the stage.
			 * @param item Popped unit, or empty if the Pumper is a source.
			 * @return Unit to push, or empty if the Pumper must not push.
			 */
			virtual Multimedia::Pipeline::Item::PointerType Process(Multimedia::Pipeline::Item::PointerType item) noexcept = 0;

		protected:
			/**
			 * @brief Worker in State::Created.
			 */
			Worker() noexcept;

			/**
			 * @brief Stage-specific drain when this worker sees EoF.
			 *
			 * Not called by the Pumper. Demux, mux and encode each
			 * implement their own flush. A filter that has nothing
			 * to drain implements an empty body.
			 */
			virtual void Flush() noexcept = 0;

			/**
			 * @brief Marks Failed and stores @p reason.
			 * @param reason Message.
			 *
			 * Does not close hoppers. Does not join a thread.
			 */
			void Fail(std::string reason) noexcept;

		private:
			Multimedia::Pipeline::State m_state;	///< Lifecycle
			std::optional<std::string> m_error;		///< Fail message
	};
}
