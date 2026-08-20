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

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/visibility.h>

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class AVCodecParameters
	 * @brief Deep-copying RAII wrapper for ::AVCodecParameters.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVCodecParameters: public AVPointer<::AVCodecParameters> {
		public:
			/**
			 * Allocates and optionally copies from @p par.
			 * @param par Source parameters (may be null).
			 */
			explicit AVCodecParameters(::AVCodecParameters* par) noexcept;

			/**
			 * Copy constructor (deep copy).
			 */
			AVCodecParameters(const AVCodecParameters& other) noexcept;

			/**
			 * Move constructor.
			 */
			AVCodecParameters(AVCodecParameters&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVCodecParameters() noexcept override;

			/**
			 * Copy assignment (deep copy).
			 */
			AVCodecParameters& operator=(const AVCodecParameters& other) noexcept;

			/**
			 * Move assignment.
			 */
			AVCodecParameters& operator=(AVCodecParameters&& other) noexcept = default;

		private:
			/**
			 * Frees parameters (avcodec_parameters_free).
			 */
			void Free() noexcept override;
	};
}
