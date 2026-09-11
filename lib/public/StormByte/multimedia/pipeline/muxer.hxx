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
	class Demuxer;
	class Encoder;
	class Muxer;
	class Remuxer;
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
		 * @brief Muxer backends. Untouched until the Backend step.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Mux {
			class Engine;

			/**
			 * @namespace Details
			 * @brief Container muxer engine.
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
	 * @brief Reserves @p encoder as an output track of @p muxer.
	 * @param encoder Live encoder.
	 * @param muxer Destination.
	 * @return @p encoder.
	 *
	 * Does not bind hoppers. Transcode / @ref Route bind
	 * @c encoder.m_out to @c muxer.m_in on the origin track.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Encoder& operator>>(Encoder& encoder, Muxer& muxer) noexcept;

	/**
	 * @brief Binds the output path of @p muxer.
	 * @param muxer Muxer.
	 * @param path Destination file.
	 * @return @p muxer.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Muxer& operator>>(Muxer& muxer, const std::filesystem::path& path) noexcept;

	/**
	 * @brief Snapshots attachments of @p file onto @p muxer.
	 * @param file Source file.
	 * @param muxer Destination.
	 * @return @p muxer.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Muxer& operator>>(const File& file, Muxer& muxer) noexcept;

	/**
	 * @brief Forwards source attachments from @p demuxer onto @p muxer.
	 * @param demuxer Open demuxer.
	 * @param muxer Destination.
	 * @return @p muxer.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Muxer& operator>>(Demuxer& demuxer, Muxer& muxer) noexcept;

	/**
	 * @class Muxer
	 * @brief Writes interleaved packets to a destination container.
	 *
	 * A @ref Step, @c final. The constructor calls @ref Step::Launch.
	 * Encoded tracks come from @c encoder >> muxer. Remux tracks come
	 * from @c remuxer >> muxer. @ref Work writes to the container, not
	 * to @ref m_out. Header write waits until reserved encode
	 * backends are open.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Muxer final: public Step {
		friend Encoder& operator>>(Encoder& encoder, Muxer& muxer) noexcept;
		friend Muxer& operator>>(Muxer& muxer, const std::filesystem::path& path) noexcept;
		friend Muxer& operator>>(const File& file, Muxer& muxer) noexcept;
		friend Muxer& operator>>(Demuxer& demuxer, Muxer& muxer) noexcept;
		friend Muxer& operator>>(Remuxer& remuxer, Muxer& muxer) noexcept;
		friend class Transcode;
		friend class Engine::Mux::Details::Container;
		friend class Engine::Mux::Details::Attachment;

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
			explicit Muxer(const Container& container) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source muxer.
			 */
			Muxer(const Muxer& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Muxer to take.
			 */
			Muxer(Muxer&& other) noexcept = delete;

			/**
			 * @brief Destructor. Closes the backend.
			 */
			~Muxer() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source muxer.
			 * @return *this.
			 */
			Muxer& operator=(const Muxer& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Muxer to take.
			 * @return *this.
			 */
			Muxer& operator=(Muxer&& other) noexcept = delete;

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
			std::optional<Property::Duration> Position() const noexcept;

			/**
			 * @brief Destination container.
			 * @return Registry container passed to the constructor.
			 */
			const Container& Destination() const noexcept;

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

			const Container* m_container;						///< Destination container
			std::unique_ptr<Engine::Mux::Engine> m_engine;		///< Format backend
			std::atomic<bool> m_closed;							///< Set by Finish / Fail
			std::atomic<std::int64_t> m_positionNs;				///< Last written Pts, or -1
	};
}
