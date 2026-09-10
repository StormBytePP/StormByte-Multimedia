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
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/visibility.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

namespace StormByte::Multimedia {
	class File;
}

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Decoder;
	class Demux;
	class Mux;
	class Transcode;

	/**
	 * @namespace Engine
	 * @brief Private backends. Public headers only forward-declare them.
	 *
	 * @ingroup multimedia_pipeline
	 */
	namespace Engine {
		/**
		 * @namespace Demux
		 * @brief Demux backends.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Demux {
			class Engine;
			/**
			 * @namespace Details
			 * @brief Container demux engine.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Details {
				class Container;
			}
		}
		/**
		 * @namespace Mux
		 * @brief Mux backends.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Mux {
			/**
			 * @namespace Details
			 * @brief Container mux engine.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Details {
				class Container;
			}
		}
	}

	/**
	 * @brief Binds @p file into @p demux and wakes the waiter. Never throws.
	 * @param file Probed snapshot.
	 * @param demux Destination.
	 * @return @p demux.
	 *
	 * Installs the format context. Does not start reading.
	 * @ref Demux::Launch is live current: call it after every
	 * decoder, remux reserve, encoder and route is wired.
	 * Connecting a half-built circuit lets copy packets reach
	 * the mux before encode tracks exist and the header is
	 * written short.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Demux& operator>>(const File& file, Demux& demux) noexcept;

	/**
	 * @brief Opens @p decoder on a stream of @p demux and binds that track.
	 * @param demux Open demuxer.
	 * @param decoder Destination.
	 * @return @p decoder.
	 *
	 * Attaches the decode backend and launches the decoder worker.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Decoder& operator>>(Demux& demux, Decoder& decoder) noexcept;

	/**
	 * @brief Forwards source attachments from @p demux onto @p mux. Never throws.
	 * @param demux Open demuxer.
	 * @param mux Destination.
	 * @return @p mux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Mux& operator>>(Demux& demux, Mux& mux) noexcept;

	/**
	 * @class Demux
	 * @brief Reads interleaved compressed packets from a File origin.
	 *
	 * A @ref Step, @c final. Unlike Decoder, Encoder, Mux and filters,
	 * the constructor does not start the worker.
	 *
	 * @c file >> demux installs the backend (@ref Pump waits on a CV).
	 * @ref Launch starts Pump after tracks are bound. Demux is the
	 * only producer that can fill the mux before the output map is
	 * complete; that is why Launch stays explicit.
	 *
	 * Input sink has zero buckets: this stage reads the File, not
	 * @ref m_in. Units that leave Pump are @c std::shared_ptr<Packet>.
	 * Empty Read is EoF. Errors are @ref Step::Fail. @ref Work stays
	 * the Step no-op.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Demux final: public Step {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Empty demuxer. Does not launch; @ref Pump waits for a source.
			 */
			Demux() noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source demuxer.
			 */
			Demux(const Demux& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Demuxer to take.
			 */
			Demux(Demux&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Demux() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source demuxer.
			 * @return *this.
			 */
			Demux& operator=(const Demux& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Demuxer to take.
			 * @return *this.
			 */
			Demux& operator=(Demux&& other) noexcept = delete;

			/**
			 * @brief true if open, not failed and not at EOF.
			 * @return Open and readable.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @}
			 */

			/**
			 * @brief Starts @ref Pump.
			 *
			 * Call after @c file >> demux and every track bind
			 * (decoder, Remux, encoder, Route::Close). Idempotent.
			 * Public on Demux; protected on @ref Step.
			 */
			void Launch() noexcept;

			/**
			 * @name Source
			 * @{
			 */

			/**
			 * @brief Whether the last read hit EOF.
			 * @return true at end of source.
			 */
			bool Eof() const noexcept;

			/**
			 * @brief Presentation time of the last pushed packet.
			 * @return Pts, or empty until a packet with Pts arrives.
			 */
			std::optional<StormByte::Multimedia::Property::Duration> Position() const noexcept;

			/**
			 * @}
			 */

			friend Demux& operator>>(const File& file, Demux& demux) noexcept;
			friend Decoder& operator>>(Demux& demux, Decoder& decoder) noexcept;
			friend Mux& operator>>(Demux& demux, Mux& mux) noexcept;
			friend class Mux;
			friend class Transcode;
			friend class Engine::Demux::Details::Container;
			friend class Engine::Mux::Details::Container;

		protected:
			/**
			 * @brief Prepare-once. No-op; the source is bound by @c file >> demux.
			 */
			void Open() noexcept override;

			/**
			 * @brief Waits for a source, then reads packets onto @ref m_out.
			 */
			void Pump() noexcept override;

			/**
			 * @brief Marks end of source after the last packet.
			 */
			void Finish() noexcept override;

		private:
			/**
			 * @brief Marks a hard error and drops the backend.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @brief Marks end of source. Called by Details::Container on AVERROR_EOF.
			 */
			void ReachedEof() noexcept;

			std::unique_ptr<Engine::Demux::Engine> m_engine;	///< Format context backend
			const File* m_file;									///< Snapshot used at open
			bool m_eof;											///< End of source
			std::mutex m_readyMutex;							///< Guards source install
			std::condition_variable m_ready;					///< Woken when m_engine is set
			std::atomic<std::int64_t> m_positionNs;				///< Last packet Pts, or -1
	};
}
