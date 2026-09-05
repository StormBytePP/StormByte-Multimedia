/*
* Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
*
* This file is part of StormByte-Multimedia.
*
* StormByte-Multimedia is dual-licensed under the following terms:
*
* 1. GNU Lesser General Public License v3.0 (or later)
*    You can redistribute it and/or modify it under the terms of the
*    GNU Lesser General Public License as published by the Free Software
*    Foundation, either version 3 of the License, or (at your option)
*    any later version.
*
* 2. Commercial license
*    Alternatively, this software may be used under the terms of a
*    commercial license agreement with the sole copyright holder
*    (David C. Manuelda <StormByte@gmail.com>).
*    Contact the copyright holder for more information.
*
* StormByte-Multimedia is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this StormByte-Multimedia. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/visibility.h>

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class AVCodecParameters
	 * @brief Deep-copying RAII wrapper for ::AVCodecParameters.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVCodecParameters: public AVPointer<::AVCodecParameters> {
		public:
			/**
			 * @brief Allocates and optionally copies from @p par.
			 * @param par Source parameters (may be null).
			 */
			explicit AVCodecParameters(::AVCodecParameters* par) noexcept;

			/**
			 * @brief Copy constructor (deep copy).
			 * @param other Source parameters.
			 */
			AVCodecParameters(const AVCodecParameters& other) noexcept;

			/**
			 * @brief Move constructor.
			 * @param other Source parameters.
			 */
			AVCodecParameters(AVCodecParameters&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~AVCodecParameters() noexcept override;

			/**
			 * @brief Copy assignment (deep copy).
			 * @param other Source parameters.
			 * @return *this.
			 */
			AVCodecParameters& operator=(const AVCodecParameters& other) noexcept;

			/**
			 * @brief Move assignment.
			 * @param other Source parameters.
			 * @return *this.
			 */
			AVCodecParameters& operator=(AVCodecParameters&& other) noexcept = default;

		private:
			/**
			 * @brief Frees parameters (avcodec_parameters_free).
			 */
			void Free() noexcept override;
	};
}
