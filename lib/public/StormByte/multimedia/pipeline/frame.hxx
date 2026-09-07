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
#include <StormByte/multimedia/property/audio.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/property/video.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <optional>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 */
namespace StormByte::Multimedia::Pipeline {
	class Decoder;
	class Encoder;
	namespace Filter {
		class Resize;
		class Watermark;
	}

	/**
	 * @class Frame
	 * @brief One decoded access unit.
	 *
	 * Move-only. Planes stay in an opaque backend buffer until Payload()
	 * is called. Attachments() is filled at receive time. Heuristics fill
	 * Video().HDR10() only, never the side-data bag.
	 * Audio() is set on audio frames; empty on video and subtitle frames.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Frame {
		public:
			/**
			 * @brief Empty frame.
			 */
			Frame() noexcept;

			/**
			 * @brief Builds a frame without a backend buffer.
			 * @param stream_index Container stream index.
			 * @param payload Owned sample / plane bytes.
			 * @param pts Presentation timestamp, if known.
			 * @param duration Frame duration, if known.
			 * @param video Video properties, if this is a video frame.
			 * @param attachments Raw side-data blobs.
			 * @param audio Audio properties, if this is an audio frame.
			 */
			Frame(int stream_index, StormByte::Buffer::FIFO payload,
				std::optional<Property::Duration> pts = std::nullopt,
				std::optional<Property::Duration> duration = std::nullopt,
				std::optional<Property::Video> video = std::nullopt,
				std::vector<class SideData> attachments = {},
				std::optional<Property::Audio> audio = std::nullopt) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Frame(const Frame&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Frame(Frame&&) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Frame() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Frame& operator=(const Frame&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Frame& operator=(Frame&&) noexcept;

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
			 * @brief Audio properties (layout, rate, channels).
			 * @return Audio, or empty.
			 */
			const std::optional<Property::Audio>& Audio() const noexcept;

			/**
			 * @brief Raw side data captured at receive.
			 * @return Blobs.
			 */
			const std::vector<class SideData>& Attachments() const noexcept;

			/**
			 * @brief Payload. Materialises planes on first call if a backend frame is held.
			 * @return FIFO.
			 */
			StormByte::Buffer::FIFO& Payload() noexcept;

			/**
			 * @brief Payload.
			 * @return FIFO. Empty until a non-const Payload() materialised it.
			 */
			const StormByte::Buffer::FIFO& Payload() const noexcept;

			friend class Decoder;
			friend Decoder& operator>>(Decoder& decoder, Frame& frame) noexcept;
			friend class Encoder;
			friend Frame& operator>>(Frame& frame, Encoder& encoder) noexcept;
			friend class Filter::Resize;
			friend class Filter::Watermark;

		private:
			class Impl;

			int m_streamIndex;
			StormByte::Buffer::FIFO m_payload;
			std::optional<Property::Duration> m_pts;
			std::optional<Property::Duration> m_duration;
			std::optional<Property::Video> m_video;
			std::optional<Property::Audio> m_audio;
			std::vector<class SideData> m_attachments;
			std::unique_ptr<Impl> m_impl;

			/**
			 * @brief Adopts a backend frame for lazy Payload().
			 * @param impl Backend holder.
			 */
			void Bind(std::unique_ptr<Impl> impl) noexcept;
	};
}
