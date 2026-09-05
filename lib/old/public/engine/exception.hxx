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

#include <StormByte/multimedia/exception.hxx>

#include <filesystem>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Exception
	 * @brief Base exception for engine components.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Exception: public StormByte::Multimedia::Exception {
		public:
			/**
			 * @tparam Args Format argument types.
			 * @param component Engine subsystem name.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			Exception(const std::string& component, std::format_string<Args...> fmt, Args&&... args):
			StormByte::Multimedia::Exception("Engine::" + component, fmt, std::forward<Args>(args)...) {}

			/**
			 * Destructor.
			 */
			virtual ~Exception() noexcept = default;
	};

	/**
	 * @class BackendNotImplemented
	 * @brief Requested backend is not available in this build.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC BackendNotImplemented: public Exception {
		public:
			/**
			 * @param backend Backend name.
			 */
			explicit BackendNotImplemented(const std::string& backend):
			Exception("Backend", "The backend '{}' is not implemented.", backend) {}
	};

	/**
	 * @class DemuxerException
	 * @brief Base for demuxer failures.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC DemuxerException: public Exception {
		public:
			/**
			 * @tparam Args Format argument types.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			DemuxerException(std::format_string<Args...> fmt, Args&&... args):
			Exception("Demuxer", fmt, std::forward<Args>(args)...) {}

			/**
			 * Destructor.
			 */
			virtual ~DemuxerException() noexcept = default;
	};

	/**
	 * @class ReadError
	 * @brief File missing or not readable.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC ReadError: public DemuxerException {
		public:
			/**
			 * @param file Path that failed.
			 */
			explicit ReadError(const std::filesystem::path& file):
			DemuxerException("File {} is not readable or it does not exist.", file.string()) {}
	};

	/**
	 * @class ContentError
	 * @brief File is not recognized as multimedia.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC ContentError: public DemuxerException {
		public:
			/**
			 * @param file Path that failed.
			 */
			explicit ContentError(const std::filesystem::path& file):
			DemuxerException("File {} is not a multimedia file.", file.string()) {}
	};
}
