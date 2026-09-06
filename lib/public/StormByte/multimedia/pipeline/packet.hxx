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

#include <StormByte/buffer/fifo.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/visibility.h>

#include <optional>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 */
namespace StormByte::Multimedia::Pipeline {
	/**
	 * @class Packet
	 * @brief One compressed access unit: owned payload plus timestamps.
	 *
	 * Move-only. The payload is a Buffer::FIFO (owned, not thread-safe).
	 * Do not read or write that FIFO from more than one thread.
	 * Pts / Dts / Duration are in nanoseconds on the stream clock, not
	 * FFmpeg ticks.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Packet {
		public:
			/**
			 * @brief Empty packet (no payload, index -1).
			 */
			Packet() noexcept;
			
			/**
			 * @brief Builds a packet.
			 * @param stream_index Container stream index.
			 * @param payload Owned compressed bytes.
			 * @param pts Presentation timestamp, if known.
			 * @param dts Decode timestamp, if known.
			 * @param duration Packet duration, if known.
			 * @param key_frame Whether this is a key frame.
			 */
			Packet(int stream_index, StormByte::Buffer::FIFO payload,
				std::optional<Property::Duration> pts = std::nullopt,
				std::optional<Property::Duration> dts = std::nullopt,
				std::optional<Property::Duration> duration = std::nullopt,
				bool key_frame = false) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Packet(const Packet&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Packet(Packet&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Packet() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Packet& operator=(const Packet&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Packet& operator=(Packet&&) noexcept = default;

			/**
			 * @brief Container stream index.
			 * @return Index.
			 */
			int StreamIndex() const noexcept;

			/**
			 * @brief Presentation timestamp.
			 * @return Pts, or empty.
			 */
			const std::optional<Property::Duration>& Pts() const noexcept;

			/**
			 * @brief Decode timestamp.
			 * @return Dts, or empty.
			 */
			const std::optional<Property::Duration>& Dts() const noexcept;

			/**
			 * @brief Packet duration.
			 * @return Duration, or empty.
			 */
			const std::optional<Property::Duration>& Duration() const noexcept;

			/**
			 * @brief Whether this is a key frame.
			 * @return true if key frame.
			 */
			bool KeyFrame() const noexcept;

			/**
			 * @brief Owned compressed payload.
			 * @return FIFO (not thread-safe).
			 */
			const StormByte::Buffer::FIFO& Payload() const noexcept;

			/**
			 * @brief Owned compressed payload (mutable).
			 * @return FIFO (not thread-safe).
			 */
			StormByte::Buffer::FIFO& Payload() noexcept;

		private:
			int m_streamIndex;								///< Container stream index
			StormByte::Buffer::FIFO m_payload;				///< Compressed bytes
			std::optional<Property::Duration> m_pts;		///< Presentation timestamp
			std::optional<Property::Duration> m_dts;		///< Decode timestamp
			std::optional<Property::Duration> m_duration;	///< Packet duration
			bool m_keyFrame;								///< Key frame
	};
}
