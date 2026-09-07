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

#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/mux.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 */
namespace StormByte::Multimedia::Pipeline {
	class Copy;

	/**
	 * @brief Binds a demux input stream onto @p copy. Never throws.
	 *
	 * Copies codec parameters, time base, language and title from the
	 * demux stream whose index is Copy::InputIndex().
	 * @param demux Open demuxer.
	 * @param copy Destination copy track.
	 * @return @p copy.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Copy& operator>>(Demux& demux, Copy& copy) noexcept;

	/**
	 * @brief Reserves Copy::Index() on @p mux as a remux track. Never throws.
	 *
	 * The track is ready immediately (no encoder open). Packets keep the
	 * demux stream index; the mux remaps them to Copy::Index().
	 * @param copy Bound copy track (must outlive the mux until the header).
	 * @param mux Destination.
	 * @return @p copy.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Copy& operator>>(Copy& copy, Mux& mux) noexcept;

	/**
	 * @class Copy
	 * @brief Output track that remuxes one demux stream without decode/encode.
	 *
	 * Construct with the output index and the demux stream index.
	 * `demux >> copy` snapshots codecpar and tags. `copy >> mux` reserves
	 * the output slot. The header waits only for reserved *encoders* to
	 * open; a bound copy is already ready.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Copy {
		public:
			/**
			 * @name Construction
			 * @{
			 */

			/**
			 * @brief Copy track from demux stream @p input_index to mux slot @p output_index.
			 * @param output_index Contiguous mux output index (>= 0).
			 * @param input_index Demux / packet stream index (>= 0).
			 */
			Copy(int output_index, int input_index) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Copy(const Copy&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Copy track to take.
			 */
			Copy(Copy&&) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Copy() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Copy& operator=(const Copy&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Copy track to take.
			 * @return *this.
			 */
			Copy& operator=(Copy&&) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name State
			 * @{
			 */

			/**
			 * @brief true if bound to a demux stream and not failed.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @brief Mux output index given at construction.
			 * @return Output index.
			 */
			int Index() const noexcept;

			/**
			 * @brief Demux stream index given at construction.
			 * @return Input index.
			 */
			int InputIndex() const noexcept;

			/**
			 * @brief Whether a hard error occurred.
			 * @return true on bind or reserve error.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text, if Failed().
			 * @return Message, or empty.
			 */
			const std::optional<std::string>& Error() const noexcept;

			/**
			 * @}
			 */

			friend Copy& operator>>(Demux& demux, Copy& copy) noexcept;
			friend Copy& operator>>(Copy& copy, Mux& mux) noexcept;
			friend class Mux;

		private:
			class Impl;

			int m_index;					///< Mux output index
			int m_input;					///< Demux stream index
			std::unique_ptr<Impl> m_impl;			///< Codecpar, time base, tags
			bool m_failed;					///< Hard error
			std::optional<std::string> m_error;		///< Failure text

			/**
			 * @brief Marks a hard error.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;
	};
}
