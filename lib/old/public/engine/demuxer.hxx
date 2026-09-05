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

#include <StormByte/multimedia/engine/implementation.hxx>
#include <StormByte/multimedia/engine/typedefs.hxx>
#include <StormByte/multimedia/engine/streams.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Demuxer
	 * @brief Opened multimedia file: path, container metadata and streams.
	 *
	 * Non-copyable; move-only. Create via Open().
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Demuxer final {
		public:
			/**
			 * Copy constructor (deleted).
			 */
			Demuxer(const Demuxer& other) = delete;

			/**
			 * Move constructor.
			 */
			Demuxer(Demuxer&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Demuxer() noexcept = default;

			/**
			 * Copy assignment (deleted).
			 */
			Demuxer& operator=(const Demuxer& other) = delete;

			/**
			 * Move assignment.
			 */
			Demuxer& operator=(Demuxer&& other) noexcept = default;

			/**
			 * @return Path of the opened file.
			 */
			const std::filesystem::path& File() const noexcept;

			/**
			 * @return Container-level metadata.
			 */
			inline const class Metadata& Metadata() const noexcept {
				return m_metadata;
			}

			/**
			 * @return Discovered streams.
			 */
			inline const class Streams& Streams() const noexcept {
				return m_streams;
			}

			/**
			 * Opens a file with the given backend.
			 * @param path File path.
			 * @param implementation Backend to use.
			 * @return Demuxer or engine exception.
			 */
			static ExpectedDemuxer Open(const std::filesystem::path& path, enum Implementation implementation) noexcept;

		private:
			std::filesystem::path m_path;		///< File path
			class Metadata m_metadata;			///< Container metadata
			class Streams m_streams;			///< Streams

			/**
			 * @param path File path.
			 * @param metadata Container metadata.
			 * @param streams Stream list.
			 */
			Demuxer(const std::filesystem::path& path, class Metadata&& metadata, class Streams&& streams) noexcept;
	};
}
