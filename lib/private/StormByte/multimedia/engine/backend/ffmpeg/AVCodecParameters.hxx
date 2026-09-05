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

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>

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

			/**
			 * @brief FFmpeg codec id.
			 * @return `AVCodecID` as int, or `AV_CODEC_ID_NONE`.
			 */
			int CodecId() const noexcept;

			/**
			 * @brief Stream bitrate in bits per second.
			 * @return Bitrate, or 0 if unknown.
			 */
			std::int64_t BitRate() const noexcept;

		private:
			/**
			 * @brief Frees parameters (avcodec_parameters_free).
			 */
			void Free() noexcept override;
	};
}
