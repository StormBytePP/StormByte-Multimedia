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

#include <StormByte/exception.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace StormByte::Multimedia
 * @brief Public Multimedia module.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Exception
	 * @brief Base exception for the Multimedia module.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Exception: public StormByte::Exception {
		public:
			/**
			 * @brief Constructs a formatted Multimedia exception.
			 * @tparam Args Format argument types.
			 * @param component Subsystem name.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			Exception(const std::string& component, std::format_string<Args...> fmt, Args&&... args):
			StormByte::Exception("Multimedia::" + component, fmt, std::forward<Args>(args)...) {}

			using StormByte::Exception::Exception;

			/**
			 * @brief Destructor.
			 */
			virtual ~Exception() noexcept = default;
	};

	/**
	 * @class CodecNotFoundException
	 * @brief Thrown when StormByte::Multimedia::Registry::FindCodec does not resolve a key.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC CodecNotFoundException: public Exception {
		public:
			/**
			 * @brief Constructs the exception for @p codec.
			 * @param codec StormByte name or FFmpeg id that was not found.
			 */
			template <typename... Args>
			CodecNotFoundException(const std::string& codec):
			Exception("Codec", "Codec {} not found", codec) {}
	};

	/**
	 * @class ContainerNotFoundException
	 * @brief Thrown when StormByte::Multimedia::Registry::FindContainer does not resolve a key.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC ContainerNotFoundException: public Exception {
		public:
			/**
			 * @brief Constructs the exception for @p container.
			 * @param container StormByte name or FFmpeg format id that was not found.
			 */
			template <typename... Args>
			ContainerNotFoundException(const std::string& container):
			Exception("Container", "Container {} not found", container) {}
	};
}
