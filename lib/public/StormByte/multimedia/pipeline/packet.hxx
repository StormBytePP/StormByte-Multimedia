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
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/visibility.h>

#include <optional>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 */
namespace StormByte::Multimedia::Pipeline {
	/**
	 * @defgroup pipeline_packet Packet
	 * @brief One compressed access unit plus optional side data.
	 *
	 * Payload is the coded AU. Attachments() is container/codec metadata
	 * that must survive encode → mux (HDR10+ for VP9/AV1, captions, etc.).
	 * Do not drop Attachments() when rebuilding a packet from bytes.
	 * @{
	 */

	/**
	 * @class Packet
	 * @brief One compressed access unit: owned payload, timestamps and side data.
	 *
	 * Move-only. The payload FIFO is owned and not thread-safe.
	 * Pts / Dts / Duration are nanoseconds on the stream clock, not FFmpeg ticks.
	 *
	 * Side data uses the same @ref SideData blobs as Frame. HDR10+ on VP9
	 * (and later AV1/VVC-in-container) lives here as SideDataKind::HdrPlus,
	 * matching AV_PKT_DATA_DYNAMIC_HDR10_PLUS. HEVC still also embeds ST 2094-40
	 * as a prefix SEI in the payload; both paths can coexist.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Packet {
		public:
			/**
			 * @name Lifetime
			 * @{
			 */

			/**
			 * @brief Empty packet (no payload, index -1).
			 */
			Packet() noexcept;

			/**
			 * @brief Builds a packet.
			 * @param stream_index Container or mux stream index.
			 * @param payload Owned compressed bytes.
			 * @param pts Presentation timestamp, if known.
			 * @param dts Decode timestamp, if known.
			 * @param duration Packet duration, if known.
			 * @param key_frame Whether this is a key frame.
			 * @param attachments Side-data blobs that must reach the muxer.
			 */
			Packet(int stream_index, StormByte::Buffer::FIFO payload,
				std::optional<Property::Duration> pts = std::nullopt,
				std::optional<Property::Duration> dts = std::nullopt,
				std::optional<Property::Duration> duration = std::nullopt,
				bool key_frame = false,
				std::vector<SideData> attachments = {}) noexcept;

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

			/** @} */

			/**
			 * @name Accessors
			 * @{
			 */

			/**
			 * @brief Container or mux stream index.
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

			/** @} */

			/**
			 * @name Side data
			 * @{
			 */

			/**
			 * @brief Side-data blobs bound to this access unit.
			 * @return Blobs (HdrPlus, captions, …). Empty when none.
			 */
			const std::vector<SideData>& Attachments() const noexcept;

			/**
			 * @brief Side-data blobs bound to this access unit (mutable).
			 * @return Blobs.
			 */
			std::vector<SideData>& Attachments() noexcept;

			/** @} */

		private:
			int m_streamIndex;								///< Container or mux stream index
			StormByte::Buffer::FIFO m_payload;				///< Compressed bytes
			std::optional<Property::Duration> m_pts;		///< Presentation timestamp
			std::optional<Property::Duration> m_dts;		///< Decode timestamp
			std::optional<Property::Duration> m_duration;	///< Packet duration
			bool m_keyFrame;								///< Key frame
			std::vector<SideData> m_attachments;			///< Packet side data (HDR10+, …)
	};

	/** @} */
}
