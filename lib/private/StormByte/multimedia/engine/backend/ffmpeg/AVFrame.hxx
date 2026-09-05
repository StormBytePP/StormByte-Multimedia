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

extern "C" {
	#include <libavutil/frame.h>
}

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class AVFrame
	 * @brief RAII wrapper for ::AVFrame.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVFrame: public AVPointer<::AVFrame> {
		friend class AVDecoder;
		friend class AVEncoder;
	public:
		/**
		 * @brief Allocates an empty frame.
		 */
		AVFrame() noexcept;

		/**
		 * @brief Copy constructor (deleted).
		 * @param other Unused.
		 */
		AVFrame(const AVFrame& other) = delete;

		/**
		 * @brief Move constructor.
		 * @param other Source frame.
		 */
		AVFrame(AVFrame&& other) noexcept = default;

		/**
		 * @brief Destructor.
		 */
		~AVFrame() noexcept override;

		/**
		 * @brief Copy assignment (deleted).
		 * @param other Unused.
		 * @return *this.
		 */
		AVFrame& operator=(const AVFrame& other) = delete;

		/**
		 * @brief Move assignment.
		 * @param other Source frame.
		 * @return *this.
		 */
		AVFrame& operator=(AVFrame&& other) noexcept = default;

		/**
		 * @brief Unreferences frame buffers (av_frame_unref).
		 */
		void Unref() noexcept;

		/**
		 * @brief Looks up side data.
		 * @param type AVFrameSideDataType value.
		 * @return Side data pointer, or nullptr.
		 */
		const AVFrameSideData* SideData(int type) const noexcept;

	private:
		/**
		 * @brief Frees the frame (av_frame_free).
		 */
		void Free() noexcept override;
	};
}
