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
#include <StormByte/multimedia/property/video.hxx>
#include <StormByte/multimedia/visibility.h>

#include <optional>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 */
namespace StormByte::Multimedia::Pipeline {
	/**
	 * @class Frame
	 * @brief One decoded access unit: payload, timestamps, video props, side data.
	 *
	 * Move-only. Payload is a Buffer::FIFO (not thread-safe). HDR10 lives in
	 * Video() when present. SideData() holds raw SEI / side data from the
	 * decoder, including MasteringDisplay / ContentLight / HdrPlus when the
	 * bitstream carried them. Heuristics fill Video().HDR10() only, never
	 * the side-data bag.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Frame {
		public:
			/**
			 * @brief Empty frame.
			 */
			Frame() noexcept;

			/**
			 * @brief Builds a frame.
			 * @param stream_index Container stream index.
			 * @param payload Owned sample / plane bytes.
			 * @param pts Presentation timestamp, if known.
			 * @param duration Frame duration, if known.
			 * @param video Video properties, if this is a video frame.
			 * @param side_data Raw side-data blobs.
			 */
			Frame(int stream_index, StormByte::Buffer::FIFO payload,
				std::optional<Property::Duration> pts = std::nullopt,
				std::optional<Property::Duration> duration = std::nullopt,
				std::optional<Property::Video> video = std::nullopt,
				std::vector<SideData> side_data = {}) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Frame(const Frame&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Frame(Frame&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Frame() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Frame& operator=(const Frame&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Frame& operator=(Frame&&) noexcept = default;

			/**
			 * @brief Container stream index.
			 * @return Index, or -1 if empty.
			 */
			int StreamIndex() const noexcept;

			/**
			 * @brief Presentation timestamp.
			 * @return Pts, or empty.
			 */
			const std::optional<Property::Duration>& Pts() const noexcept;

			/**
			 * @brief Frame duration.
			 * @return Duration, or empty.
			 */
			const std::optional<Property::Duration>& Duration() const noexcept;

			/**
			 * @brief Video properties (includes HDR10 when set).
			 * @return Video, or empty.
			 */
			const std::optional<Property::Video>& Video() const noexcept;

			/**
			 * @brief Raw side data from the decoder.
			 * @return Blobs.
			 */
			const std::vector<SideData>& SideData() const noexcept;

			/**
			 * @brief Owned payload.
			 * @return FIFO.
			 */
			const StormByte::Buffer::FIFO& Payload() const noexcept;

			/**
			 * @brief Owned payload (mutable).
			 * @return FIFO.
			 */
			StormByte::Buffer::FIFO& Payload() noexcept;

		private:
			int m_streamIndex;					///< Stream index
			StormByte::Buffer::FIFO m_payload;			///< Samples / planes
			std::optional<Property::Duration> m_pts;		///< Presentation timestamp
			std::optional<Property::Duration> m_duration;		///< Frame duration
			std::optional<Property::Video> m_video;			///< Video properties
			std::vector<class SideData> m_sideData;			///< Raw side data
	};
}
