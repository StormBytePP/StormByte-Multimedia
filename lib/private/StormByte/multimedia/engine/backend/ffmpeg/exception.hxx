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

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class Exception
	 * @brief Base for FFmpeg backend errors.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Exception: public Multimedia::Exception {
		public:
			/**
			 * @brief Constructs a formatted AV exception.
			 * @tparam Args Format argument types.
			 * @param component Subsystem label.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			Exception(const std::string& component, std::format_string<Args...> fmt, Args&&... args):
			StormByte::Exception("AV::" + component, fmt, std::forward<Args>(args)...) {}

			using Multimedia::Exception::Exception;

			/**
			 * @brief Destructor.
			 */
			virtual ~Exception() noexcept = default;
	};

	/**
	 * @class BSFError
	 * @brief Bitstream filter failure.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE BSFError: public Exception {
		public:
			/**
			 * @brief Constructs a BSF error.
			 * @tparam Args Format argument types.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			BSFError(std::format_string<Args...> fmt, Args&&... args):
			Exception("BSF: ", fmt, std::forward<Args>(args)...) {}

			using FFmpeg::Exception::Exception;
	};

	/**
	 * @class DecoderError
	 * @brief Decoder open or process failure.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE DecoderError: public Exception {
		public:
			/**
			 * @brief Constructs a decoder error.
			 * @tparam Args Format argument types.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			DecoderError(std::format_string<Args...> fmt, Args&&... args):
			Exception("Decoder: ", fmt, std::forward<Args>(args)...) {}

			using FFmpeg::Exception::Exception;
	};

	/**
	 * @class EncoderError
	 * @brief Encoder open or process failure.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE EncoderError: public Exception {
		public:
			/**
			 * @brief Constructs an encoder error.
			 * @tparam Args Format argument types.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			EncoderError(std::format_string<Args...> fmt, Args&&... args):
			Exception("Encoder: ", fmt, std::forward<Args>(args)...) {}

			using FFmpeg::Exception::Exception;
	};
}
