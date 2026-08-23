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
	#include <libavcodec/packet.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class AVPacket
	 * @brief RAII wrapper for ::AVPacket.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVPacket: public AVPointer<::AVPacket> {
		friend class AVBSF;
		friend class AVDecoder;
		friend class AVEncoder;
		friend class AVFormatContext;
	public:
		/**
		 * Allocates an empty packet.
		 */
		AVPacket() noexcept;

		/**
		 * Copy constructor (deleted).
		 */
		AVPacket(const AVPacket& other) = delete;

		/**
		 * Move constructor.
		 */
		AVPacket(AVPacket&& other) noexcept = default;

		/**
		 * Destructor.
		 */
		~AVPacket() noexcept override;

		/**
		 * Copy assignment (deleted).
		 */
		AVPacket& operator=(const AVPacket& other) = delete;

		/**
		 * Move assignment.
		 */
		AVPacket& operator=(AVPacket&& other) noexcept = default;

		/**
		 * @return New packet referencing the same data (av_packet_ref).
		 */
		FFmpeg::AVPacket Ref() const noexcept;

		/**
		 * Unreferences packet data (av_packet_unref).
		 */
		void Unref() noexcept;

		/**
		 * @return stream_index, or -1 if empty.
		 */
		int StreamIndex() const noexcept;

	private:
		/**
		 * Frees the packet (av_packet_free).
		 */
		void Free() noexcept override;
	};
}
