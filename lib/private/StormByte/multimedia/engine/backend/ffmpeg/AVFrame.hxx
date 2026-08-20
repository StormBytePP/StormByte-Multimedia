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

extern "C" {
	#include <libavutil/frame.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
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
		 * Allocates an empty frame.
		 */
		AVFrame() noexcept;

		/**
		 * Copy constructor (deleted).
		 */
		AVFrame(const AVFrame& other) = delete;

		/**
		 * Move constructor.
		 */
		AVFrame(AVFrame&& other) noexcept = default;

		/**
		 * Destructor.
		 */
		~AVFrame() noexcept override;

		/**
		 * Copy assignment (deleted).
		 */
		AVFrame& operator=(const AVFrame& other) = delete;

		/**
		 * Move assignment.
		 */
		AVFrame& operator=(AVFrame&& other) noexcept = default;

		/**
		 * Unreferences frame buffers (av_frame_unref).
		 */
		void Unref() noexcept;

		/**
		 * @param type AVFrameSideDataType value.
		 * @return Side data pointer, or nullptr.
		 */
		const AVFrameSideData* SideData(int type) const noexcept;

	private:
		/**
		 * Frees the frame (av_frame_free).
		 */
		void Free() noexcept override;
	};
}
