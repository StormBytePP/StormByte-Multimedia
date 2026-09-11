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
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/visibility.h>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Demux;
	class Encoder;
	class Mux;
	class Remux;
	class Transcode;

	/**
	 * @namespace Engine
	 * @brief Private backends. Public headers only forward-declare them.
	 *
	 * @ingroup multimedia_pipeline
	 */
	namespace Engine {
		/**
		 * @namespace Mux
		 * @brief Mux backends.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Mux {
			class Engine;
			/**
			 * @namespace Details
			 * @brief Container mux engine.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Details {
				class Container;
				class Attachment;
			}
		}
	}

	/**
	 * @brief Reserves @p encoder as an output track of @p mux.
	 * @param encoder Live encoder.
	 * @param mux Destination.
	 * @return @p encoder.
	 *
	 * Does not bind hoppers. Transcode / @ref Route bind
	 * @c encoder.m_out to @c mux.m_in on the origin track.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Encoder& operator>>(Encoder& encoder, Mux& mux) noexcept;

	/**
	 * @brief Binds the output path of @p mux.
	 * @param mux Muxer.
	 * @param path Destination file.
	 * @return @p mux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Mux& operator>>(Mux& mux, const std::filesystem::path& path) noexcept;

	/**
	 * @brief Snapshots attachments of @p file onto @p mux.
	 * @param file Source file.
	 * @param mux Destination.
	 * @return @p mux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Mux& operator>>(const File& file, Mux& mux) noexcept;

	/**
	 * @brief Forwards source attachments from @p demux onto @p mux.
	 * @param demux Open demuxer.
	 * @param mux Destination.
	 * @return @p mux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Mux& operator>>(Demux& demux, Mux& mux) noexcept;

	/**
	 * @class Mux
	 * @brief Writes interleaved packets to a destination container.
	 *
	 * A @ref Step, @c final. The constructor calls @ref Step::Launch.
	 * Encoded tracks come from @c encoder >> mux. Remux tracks come
	 * from @c remux >> mux. @ref Work writes to the container, not
	 * to @ref m_out. Header write waits until reserved encode
	 * backends are open.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Mux final: public Step {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Muxer for @p container. Destination path is bound later.
			 * @param container Writable registry container.
			 *
			 * Starts the worker. @ref Pop waits until Bind creates buckets.
			 */
			explicit Mux(const StormByte::Multimedia::Container& container) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source muxer.
			 */
			Mux(const Mux& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Muxer to take.
			 */
			Mux(Mux&& other) noexcept = delete;

			/**
			 * @brief Destructor. Closes the backend.
			 */
			~Mux() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source muxer.
			 * @return *this.
			 */
			Mux& operator=(const Mux& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Muxer to take.
			 * @return *this.
			 */
			Mux& operator=(Mux&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @brief true if the destination is bound and the muxer has not failed.
			 * @return Open and writable.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @brief true after @ref Finish flushed the trailer, or after @ref Fail.
			 * @return Muxer will not accept more packets.
			 */
			bool Closed() const noexcept;

			/**
			 * @brief Presentation time of the last packet written.
			 * @return Pts, or empty until a packet with Pts is written.
			 */
			std::optional<StormByte::Multimedia::Property::Duration> Position() const noexcept;

			/**
			 * @brief Destination container.
			 * @return Registry container passed to the constructor.
			 */
			const StormByte::Multimedia::Container& Destination() const noexcept;

			friend Encoder& operator>>(Encoder& encoder, Mux& mux) noexcept;
			friend Mux& operator>>(Mux& mux, const std::filesystem::path& path) noexcept;
			friend Mux& operator>>(const File& file, Mux& mux) noexcept;
			friend Mux& operator>>(Demux& demux, Mux& mux) noexcept;
			friend Mux& operator>>(Remux& remux, Mux& mux) noexcept;
			friend class Transcode;
			friend class Engine::Mux::Details::Container;
			friend class Engine::Mux::Details::Attachment;

		protected:
			/**
			 * @brief Prepare-once. Does not Fail if the path is still unbound.
			 */
			void Open() noexcept override;

			/**
			 * @brief Writes one packet to the container. Does not touch @ref m_out.
			 * @param item Incoming packet.
			 */
			void Work(std::shared_ptr<Item> item) noexcept override;

			/**
			 * @brief Flushes leftover packets and the trailer.
			 */
			void Finish() noexcept override;

		private:
			/**
			 * @brief Marks a hard error and closes the backend.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			const StormByte::Multimedia::Container* m_container;	///< Destination container
			std::unique_ptr<Engine::Mux::Engine> m_engine;			///< Format backend
			std::atomic<bool> m_closed;								///< Set by Finish / Fail
			std::atomic<std::int64_t> m_positionNs;					///< Last written Pts, or -1
	};
}
