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
#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <optional>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	/**
	 * @class Packet
	 * @brief One compressed access unit.
	 *
	 * In libav that is an @c AVPacket: compressed bytes for one
	 * picture, one audio block or one subtitle event. In Multimedia
	 * it is what packet filters, @ref Remuxer and @ref Muxer see —
	 * origin track, timestamps, payload and side data — without
	 * opening a codec.
	 *
	 * @see Item
	 * @see Frame
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Packet: public Item {
		friend class Backend::Pipeline::Packet;
		friend class Decoder;
		friend class Demuxer;
		friend class Encoder;
		friend class Filter::FFmpeg;
		friend class Muxer;

		public:
			/**
			 * @name Construction
			 * @{
			 */

			/**
			 * @brief Empty packet (no payload, track -1, type Unknown).
			 */
			Packet() noexcept;

			/**
			 * @brief Builds a packet.
			 * @param track Origin container stream index.
			 * @param type Media of this access unit. Not Copy.
			 * @param producer Step that created this unit.
			 * @param payload Owned compressed bytes.
			 * @param pts Presentation timestamp, if known.
			 * @param dts Decode timestamp, if known.
			 * @param duration Packet duration, if known.
			 * @param key_frame Whether this is a key frame.
			 * @param attachments Side-data blobs that must reach the muxer.
			 */
			Packet(int track, enum Type type, enum Producer producer,
				StormByte::Buffer::FIFO payload,
				std::optional<Property::Duration> pts = std::nullopt,
				std::optional<Property::Duration> dts = std::nullopt,
				std::optional<Property::Duration> duration = std::nullopt,
				bool key_frame = false,
				std::vector<SideData> attachments = {}) noexcept;

			/**
			 * @brief Move constructor.
			 * @param other Packet to take.
			 *
			 * @p other becomes the empty sentinel.
			 */
			Packet(Packet&& other) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Packet() noexcept override;

			/**
			 * @brief Move assignment.
			 * @param other Packet to take.
			 * @return *this.
			 *
			 * @p other becomes the empty sentinel.
			 */
			Packet& operator=(Packet&& other) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Timing
			 * @{
			 */

			/**
			 * @brief Presentation timestamp on the stream clock.
			 * @return Pts, or empty.
			 */
			inline const std::optional<Property::Duration>& Pts() const noexcept {
				return m_pts;
			}

			/**
			 * @brief Decode timestamp on the stream clock.
			 * @return Dts, or empty.
			 */
			inline const std::optional<Property::Duration>& Dts() const noexcept {
				return m_dts;
			}

			/**
			 * @brief Packet duration on the stream clock.
			 * @return Duration, or empty.
			 */
			inline const std::optional<Property::Duration>& Duration() const noexcept {
				return m_duration;
			}

			/**
			 * @brief Whether this is a key frame / key packet.
			 * @return true if marked as a key frame.
			 */
			inline bool KeyFrame() const noexcept {
				return m_keyFrame;
			}

			/**
			 * @}
			 */

			/**
			 * @name Payload
			 * @{
			 */

			/**
			 * @brief Owned compressed payload.
			 * @return FIFO (not thread-safe).
			 */
			inline const StormByte::Buffer::FIFO& Payload() const noexcept {
				return m_payload;
			}

			/**
			 * @brief Owned compressed payload (mutable).
			 * @return FIFO (not thread-safe).
			 */
			inline StormByte::Buffer::FIFO& Payload() noexcept {
				return m_payload;
			}

			/**
			 * @brief Side-data blobs bound to this access unit.
			 * @return Blobs (HdrPlus, captions, …). Empty when none.
			 */
			inline const std::vector<SideData>& Attachments() const noexcept {
				return m_attachments;
			}

			/**
			 * @brief Side-data blobs bound to this access unit (mutable).
			 * @return Blobs.
			 */
			inline std::vector<SideData>& Attachments() noexcept {
				return m_attachments;
			}

			/**
			 * @}
			 */

		private:
			/**
			 * @name Construction
			 * @{
			 */

			/**
			 * @brief Deep copy (metadata, FIFO and cloned @c AVPacket).
			 * @param other Source packet.
			 *
			 * Private: a public copy would duplicate the compressed AU.
			 * Only @ref Filter::FFmpeg and the producing steps may clone.
			 */
			Packet(const Packet& other) noexcept;

			/**
			 * @brief Deep copy assignment.
			 * @param other Source packet.
			 * @return *this.
			 */
			Packet& operator=(const Packet& other) noexcept;

			/**
			 * @}
			 */

			/**
			 * @brief Adopts a backend packet.
			 * @param backend Backend holder.
			 */
			void Bind(std::unique_ptr<Backend::Pipeline::Packet> backend) noexcept;

			/**
			 * @brief Turns this unit into the empty sentinel.
			 */
			void BecomeEmpty() noexcept;

			StormByte::Buffer::FIFO m_payload;								///< Compressed bytes
			std::optional<Property::Duration> m_pts;						///< Presentation timestamp
			std::optional<Property::Duration> m_dts;						///< Decode timestamp
			std::optional<Property::Duration> m_duration;					///< Packet duration
			bool m_keyFrame;												///< Key frame
			std::vector<SideData> m_attachments;							///< Packet side data
			std::unique_ptr<Backend::Pipeline::Packet> m_backend;			///< Backend holder
	};
}
