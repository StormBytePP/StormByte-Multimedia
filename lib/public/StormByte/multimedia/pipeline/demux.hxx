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
 */
namespace StormByte::Multimedia::Pipeline {
	class Demux;

	/**
	 * @brief Opens @p file into @p demux. Never throws.
	 * @param file Probed snapshot.
	 * @param demux Destination (replaced).
	 * @return @p demux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Demux& operator>>(const File& file, Demux& demux) noexcept;

	/**
	 * @brief Reads one packet into @p packet. Never throws.
	 * @param demux Source demuxer.
	 * @param packet Replaced on success.
	 * @return @p demux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Demux& operator>>(Demux& demux, Packet& packet) noexcept;

	/**
	 * @class Demux
	 * @brief Reads interleaved compressed packets from a File origin.
	 *
	 * Open and read go through operator>>. Hard errors set Failed(); EOF
	 * sets Eof(). Neither throws. operator>> on a failed or ended Demux
	 * is a no-op. operator bool() is false on fail or EOF so
	 * `while (demux >> packet)` stops. Check Failed() when the run finishes.
	 *
	 * One Demux per File if the origin is a Consumer. File::Duration() on a
	 * Consumer drains the ring; do not scan duration before demuxing a buffer.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Demux {
		public:
			/**
			 * @brief Empty demuxer (not open).
			 */
			Demux() noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Demux(const Demux&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Demux(Demux&&) noexcept;

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
			 * @return *this.
			 */
			Demux& operator=(Demux&&) noexcept;

			/**
			 * @brief true if open, not failed and not at EOF.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @brief Whether a hard error occurred.
			 * @return true on open/read error.
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

			friend Demux& operator>>(const File& file, Demux& demux) noexcept;
			friend Demux& operator>>(Demux& demux, Packet& packet) noexcept;

		private:
			class Impl;
			std::unique_ptr<Impl> m_impl;				///< FFmpeg context
			bool m_failed;						///< Hard error
			bool m_eof;						///< End of source
			std::optional<std::string> m_error;			///< Failure text

			/**
			 * @brief Marks a hard error.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;
	};
}
