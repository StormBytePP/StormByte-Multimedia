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

#include <StormByte/buffer/consumer.hxx>
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/metadata/file.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/stream.hxx>
#include <StormByte/multimedia/typedefs.hxx>

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>

/**
 * @namespace StormByte::Multimedia
 * @brief Public multimedia types: codecs, containers, streams and files.
 */
namespace StormByte::Multimedia {
	class Origin;

	/**
	 * @class File
	 * @brief Snapshot of a media source: path or Consumer, container, streams and tags.
	 *
	 * File is move-only. Open() probes with private FFmpeg RAII and drops the
	 * demuxer before return. A Consumer origin is kept; its Ring is exclusive
	 * to this File for its lifetime.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC File {
		public:
			/**
			 * @brief Copy constructor (deleted).
			 */
			File(const File&) = delete;

			/**
			 * @brief Move constructor.
			 */
			File(File&&) noexcept;

			/**
			 * @brief Destructor.
			 */
			~File() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			File& operator=(const File&) = delete;

			/**
			 * @brief Move assignment (deleted: Container is a reference).
			 * @return *this.
			 */
			File& operator=(File&&) = delete;

			/**
			 * @brief Filesystem path passed to Open, or empty if the origin is a Consumer.
			 * @return Path.
			 */
			const std::filesystem::path& Path() const noexcept;

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
			 * @brief Container duration.
			 * @return Duration, or empty if it cannot be determined.
			 *
			 * Returns the header value, the duration passed to Open, or a value
			 * cached after the first scan. The first call may read the whole source
			 * when Open was used without a duration: even if the container header
			 * has a duration, streams that lack one are filled from packet
			 * timestamps. After a successful scan the result is reused on this
			 * instance. If Open(..., duration) was used, this is that value, there
			 * is no I/O, and stream durations stay as probed.
			 */
			const std::optional<Property::Duration>& Duration() const noexcept;

			/**
			 * @brief Opens and probes @p path.
			 * @param path Media file.
			 * @return Snapshot or FileOpenErrorException.
			 */
			static ExpectedFile Open(const std::filesystem::path& path) noexcept;

			/**
			 * @brief Opens and probes @p path with an already known duration.
			 * @param path Media file.
			 * @param duration Authoritative container duration in nanoseconds.
			 * @return Snapshot or FileOpenErrorException.
			 */
			static ExpectedFile Open(const std::filesystem::path& path,
				std::chrono::nanoseconds duration) noexcept;

			/**
			 * @brief Opens and probes a Consumer.
			 * @param consumer Shared ring handle (copied and kept).
			 * @return Snapshot or FileOpenErrorException.
			 */
			static ExpectedFile Open(StormByte::Buffer::Consumer consumer) noexcept;

			/**
			 * @brief Opens and probes a Consumer with an already known duration.
			 * @param consumer Shared ring handle (copied and kept).
			 * @param duration Authoritative container duration in nanoseconds.
			 * @return Snapshot or FileOpenErrorException.
			 */
			static ExpectedFile Open(StormByte::Buffer::Consumer consumer,
				std::chrono::nanoseconds duration) noexcept;

		private:
			std::unique_ptr<Origin> m_origin;						///< Path or Consumer
			const class Container& m_container;						///< Registry container
			mutable Multimedia::Streams m_streams;					///< Probed streams
			Metadata::File m_metadata;								///< Container tags
			mutable std::optional<Property::Duration> m_duration;	///< Container duration
			mutable bool m_durationResolved;						///< Caller-supplied or scan done

			/**
			 * @brief Snapshot constructor.
			 * @param origin Path or Consumer.
			 * @param container Registry container.
			 * @param streams Probed streams.
			 * @param metadata Container tags.
			 * @param duration Container duration.
			 * @param durationResolved true if Duration() must not scan.
			 */
			File(std::unique_ptr<Origin> origin, const class Container& container,
				Multimedia::Streams streams, Metadata::File metadata,
				std::optional<Property::Duration> duration, bool durationResolved) noexcept;

			/**
			 * @brief Shared Open implementation.
			 * @param origin Path or Consumer.
			 * @param duration Caller-supplied duration, if any.
			 * @return Snapshot or FileOpenErrorException.
			 */
			static ExpectedFile Open(std::unique_ptr<Origin> origin,
				std::optional<std::chrono::nanoseconds> duration) noexcept;

			/**
			 * @brief Packet scan for container and missing stream durations.
			 */
			void ResolveDuration() const noexcept;
	};
}
