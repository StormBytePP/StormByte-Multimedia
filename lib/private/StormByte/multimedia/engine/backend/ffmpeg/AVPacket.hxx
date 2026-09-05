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

extern "C" {
	#include <libavcodec/packet.h>
}

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
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
		 * @brief Allocates an empty packet.
		 */
		AVPacket() noexcept;

		/**
		 * @brief Copy constructor (deleted).
		 * @param other Unused.
		 */
		AVPacket(const AVPacket& other) = delete;

		/**
		 * @brief Move constructor.
		 * @param other Source packet.
		 */
		AVPacket(AVPacket&& other) noexcept = default;

		/**
		 * @brief Destructor.
		 */
		~AVPacket() noexcept override;

		/**
		 * @brief Copy assignment (deleted).
		 * @param other Unused.
		 * @return *this.
		 */
		AVPacket& operator=(const AVPacket& other) = delete;

		/**
		 * @brief Move assignment.
		 * @param other Source packet.
		 * @return *this.
		 */
		AVPacket& operator=(AVPacket&& other) noexcept = default;

		/**
		 * @brief New packet referencing the same data (av_packet_ref).
		 * @return Referenced packet.
		 */
		FFmpeg::AVPacket Ref() const noexcept;

		/**
		 * @brief Unreferences packet data (av_packet_unref).
		 */
		void Unref() noexcept;

		/**
		 * @brief Packet stream index.
		 * @return stream_index, or -1 if empty.
		 */
		int StreamIndex() const noexcept;

	private:
		/**
		 * @brief Frees the packet (av_packet_free).
		 */
		void Free() noexcept override;
	};
}
