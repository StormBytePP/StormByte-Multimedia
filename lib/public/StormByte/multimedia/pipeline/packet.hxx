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

#include <memory>
#include <optional>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 */
namespace StormByte::Multimedia::Pipeline {
	namespace Filter {
		class FFmpeg;	///< Sole filter friend of the private copy.
	}

	/**
	 * @namespace Engine
	 * @brief Private backends. Public headers only forward-declare them.
	 */
	namespace Engine {
		/**
		 * @namespace Packet
		 * @brief Compressed-AU backend behind the public Packet type.
		 */
		namespace Packet {
			class Engine;	///< Opaque holder of the backend @c AVPacket.
		}
	}

	/**
	 * @class Packet
	 * @brief One compressed access unit: owned payload, timestamps and side data.
	 *
	 * Public API is move-only. There is no public @c Clone().
	 * Copy constructor and copy assignment stay private and clone
	 * metadata, the payload FIFO and the backend @c AVPacket when
	 * @ref StormByte::Multimedia::Pipeline::Engine::Packet::Engine
	 * exists. @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg::Clone
	 * is how a filter keeps a copy.
	 *
	 * Pts / Dts / Duration are nanoseconds on the stream clock, not
	 * FFmpeg ticks. Side data uses the same @ref SideData blobs as
	 * Frame. After
	 * @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg::Replace
	 * the previous backend is released and the FIFO is cleared.
	 *
	 * Destructor and move are out of line so this header can forward-declare
	 * @ref Engine::Packet::Engine. The .cxx includes the engine definition.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Packet {
		public:
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
			 * @brief Move constructor.
			 * @param other Packet to take.
			 */
			Packet(Packet&& other) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Packet() noexcept;

			/**
			 * @brief Move assignment.
			 * @param other Packet to take.
			 * @return *this.
			 */
			Packet& operator=(Packet&& other) noexcept;

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

		friend class Filter::FFmpeg;

		private:
			/**
			 * @brief Deep copy (metadata, FIFO and cloned @c AVPacket).
			 * @param other Source packet.
			 *
			 * Private: a public copy would duplicate the compressed AU.
			 * Only @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg
			 * may clone, via @ref Filter::FFmpeg::Clone.
			 */
			Packet(const Packet& other) noexcept;

			/**
			 * @brief Deep copy assignment.
			 * @param other Source packet.
			 * @return *this.
			 */
			Packet& operator=(const Packet& other) noexcept;

			/**
			 * @brief Adopts a backend packet.
			 * @param engine Backend holder.
			 */
			void Bind(std::unique_ptr<Engine::Packet::Engine> engine) noexcept;

			int m_streamIndex;								///< Container or mux stream index
			StormByte::Buffer::FIFO m_payload;				///< Compressed bytes
			std::optional<Property::Duration> m_pts;		///< Presentation timestamp
			std::optional<Property::Duration> m_dts;		///< Decode timestamp
			std::optional<Property::Duration> m_duration;	///< Packet duration
			bool m_keyFrame;								///< Key frame
			std::vector<SideData> m_attachments;			///< Packet side data (HDR10+, …)
			std::unique_ptr<Engine::Packet::Engine> m_engine;	///< Backend holder
	};
}
