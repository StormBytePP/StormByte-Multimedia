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
#include <StormByte/multimedia/metadata/file.hxx>
#include <StormByte/multimedia/stream.hxx>
#include <StormByte/multimedia/typedefs.hxx>

#include <filesystem>

/**
 * @namespace StormByte::Multimedia
 * @brief Public multimedia types: codecs, containers, streams and files.
 */
namespace StormByte::Multimedia {
	/**
	 * @class File
	 * @brief Snapshot of an existing media file: path, container, streams and tags.
	 *
	 * Open() probes with private FFmpeg RAII and drops it before return.
	 * File stores no backend state. Copies share registry Codec/Container refs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC File {
		public:
			/**
			 * @brief Copy constructor.
			 */
			File(const File&) = default;

			/**
			 * @brief Move constructor.
			 */
			File(File&&) = default;

			/**
			 * @brief Destructor.
			 */
			~File() = default;

			/**
			 * @brief Copy assignment.
			 * @return *this.
			 */
			File& operator=(const File&) = default;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			File& operator=(File&&) = default;

			/**
			 * @brief Filesystem path passed to Open.
			 * @return Path.
			 */
			const std::filesystem::path& Path() const noexcept { return m_path; }

			/**
			 * @brief Detected container.
			 * @return Registry container.
			 */
			const class Container& Container() const noexcept { return m_container; }

			/**
			 * @brief Streams in container order.
			 * @return Immutable list.
			 */
			const Multimedia::Streams& Streams() const noexcept { return m_streams; }

			/**
			 * @brief Container-level tags captured at Open.
			 * @return Metadata snapshot.
			 */
			const Metadata::File& Metadata() const noexcept { return m_metadata; }

			/**
			 * @brief Opens and probes @p path.
			 * @param path Media file.
			 * @return Snapshot or FileOpenErrorException.
			 */
			static ExpectedFile Open(const std::filesystem::path& path) noexcept;

		private:
			std::filesystem::path m_path;		///< Source path
			const class Container& m_container;	///< Registry container
			Multimedia::Streams m_streams;		///< Probed streams
			Metadata::File m_metadata;		///< Container tags

			/**
			 * @brief Snapshot constructor.
			 * @param path Source path.
			 * @param container Registry container.
			 * @param streams Probed streams.
			 * @param metadata Container tags.
			 */
			File(const std::filesystem::path& path, const class Container& container,
				Multimedia::Streams streams, Metadata::File metadata) noexcept
			: m_path(path), m_container(container), m_streams(std::move(streams)), m_metadata(std::move(metadata)) {}
	};
}
