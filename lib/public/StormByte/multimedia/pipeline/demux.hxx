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

#include <StormByte/multimedia/pipeline/filters/chain.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
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
	class Copy;
	class Decoder;
	class Demux;
	class Mux;

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
	}

	/**
	 * @brief Opens @p file into @p demux. Never throws.
	 * @param file Probed snapshot.
	 * @param demux Destination (replaced).
	 * @return @p demux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Demux& operator>>(const File& file, Demux& demux) noexcept;

	/**
	 * @brief Reads one packet into @p packet after the filter chain. Never throws.
	 * @param demux Source demuxer.
	 * @param packet Replaced on success.
	 * @return @p demux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Demux& operator>>(Demux& demux, class Packet& packet) noexcept;

	/**
	 * @brief Opens @p decoder on a stream of @p demux. Never throws.
	 * @param demux Open demuxer.
	 * @param decoder Destination.
	 * @return @p decoder.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Decoder& operator>>(Demux& demux, Decoder& decoder) noexcept;

	/**
	 * @brief Binds a demux input stream onto @p copy. Never throws.
	 * @param demux Open demuxer.
	 * @param copy Destination copy track.
	 * @return @p copy.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Copy& operator>>(Demux& demux, Copy& copy) noexcept;

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
	 * Public entry point. Open and Read live in Details::Container.
	 * @ref Filter::Chain goes in Pipe(). @c demux >> packet calls
	 * @ref Filter::Chain::Call with @ref Filter::Origin::Demux.
	 * @ref ReachedEof also @ref Filter::Chain::Eof that origin.
	 * A null pipe is identity.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Demux {
		public:
			/**
			 * @name Lifetime
			 * @{
			 */

			/**
			 * @brief Empty demuxer (not open).
			 * @param pipe Shared filter list, or null.
			 */
			explicit Demux(std::shared_ptr<Filter::Chain> pipe = nullptr) noexcept;

			/**
			 * @brief Demuxer bound to an existing chain (non-owning alias).
			 * @param pipe Live chain.
			 */
			explicit Demux(Filter::Chain& pipe) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Demux(const Demux&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Demuxer to take.
			 */
			Demux(Demux&& other) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Demux() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Demux& operator=(const Demux&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Demuxer to take.
			 * @return *this.
			 */
			Demux& operator=(Demux&& other) noexcept;

			/**
			 * @brief true if open, not failed and not at EOF.
			 * @return Open, not @ref Failed and not @ref Eof.
			 */
			explicit operator bool() const noexcept;

			/** @} */

			/**
			 * @name Status
			 * @{
			 */

			/**
			 * @brief Whether a hard error occurred.
			 * @return true on open/read/filter error.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Whether the last read hit EOF.
			 * @return true at end of source.
			 */
			bool Eof() const noexcept;

			/**
			 * @brief Failure text, if Failed().
			 * @return Message, or empty.
			 */
			const std::optional<std::string>& Error() const noexcept;

			/** @} */

			/**
			 * @name Pipe
			 * @{
			 */

			/**
			 * @brief Shared filter chain, or null.
			 * @return Pipe.
			 */
			std::shared_ptr<Filter::Chain>& Pipe() noexcept;

			/**
			 * @brief Shared filter chain, or null.
			 * @return Pipe.
			 */
			const std::shared_ptr<Filter::Chain>& Pipe() const noexcept;

			/**
			 * @brief Replaces the shared chain.
			 * @param pipe New list, or null.
			 */
			void Pipe(std::shared_ptr<Filter::Chain> pipe) noexcept;

			/**
			 * @brief Aliases a live chain (non-owning).
			 * @param pipe Live list.
			 */
			void Pipe(Filter::Chain& pipe) noexcept;

			/** @} */

			friend Demux& operator>>(const File& file, Demux& demux) noexcept;
			friend Demux& operator>>(Demux& demux, class Packet& packet) noexcept;
			friend Decoder& operator>>(Demux& demux, Decoder& decoder) noexcept;
			friend Copy& operator>>(Demux& demux, Copy& copy) noexcept;
			friend Mux& operator>>(Demux& demux, Mux& mux) noexcept;
			friend class Engine::Demux::Details::Container;

		private:
			std::unique_ptr<Engine::Demux::Engine> m_engine;	///< Format context backend
			const File* m_file = nullptr;						///< Snapshot used at open
			std::shared_ptr<Filter::Chain> m_pipe;				///< Shared filter list
			bool m_failed;										///< Hard error
			bool m_eof;											///< End of source
			std::optional<std::string> m_error;					///< Failure text

			/**
			 * @brief Marks a hard error and drops the backend.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @brief Marks end of source and @ref Filter::Chain::Eof Demux.
			 */
			void ReachedEof() noexcept;
	};
}
