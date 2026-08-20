/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/multimedia/exception.hxx>

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class Exception
	 * @brief Base for FFmpeg backend errors (AV::…).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Exception: public Multimedia::Exception {
		public:
			/**
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
			 * Destructor.
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
	 * @brief Decoder open/process failure.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE DecoderError: public Exception {
		public:
			/**
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
	 * @brief Encoder open/process failure.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE EncoderError: public Exception {
		public:
			/**
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
