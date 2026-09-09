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

#include <StormByte/multimedia/backend/ffmpeg/AVEncoder.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/engine.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>

#include <cstdint>
#include <deque>
#include <memory>
#include <optional>

extern "C" {
	#include <libavutil/rational.h>
}

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Encoder::Details
 * @brief Per-media encode engines.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Encoder::Details {
	/**
	 * @class Video
	 * @brief Video encode backend: table row, fps time_base, HDR side data, SendFrame.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Video final: public Encoder::Engine {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Default constructor.
			 */
			Video() noexcept;

			/**
			 * @brief Destructor.
			 */
			~Video() noexcept override = default;

			/**
			 * @brief Copy constructor.
			 * @param other Source engine.
			 */
			Video(const Video& other) = delete;

			/**
			 * @brief Copy assignment.
			 * @param other Source engine.
			 * @return *this.
			 */
			Video& operator=(const Video& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Engine to take.
			 */
			Video(Video&& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Engine to take.
			 * @return *this.
			 */
			Video& operator=(Video&& other) noexcept = default;

			/**
			 * @}
			 */

			/**
			 * @brief Whether the FFmpeg encoder is open.
			 * @return true after a successful Open().
			 */
			bool IsOpen() const noexcept override;

			/**
			 * @brief Opens the video encoder from the first frame.
			 * @param owner Public encoder.
			 * @param frame First video frame.
			 * @return false if owner.Fail() was called.
			 */
			bool Open(class StormByte::Multimedia::Pipeline::Encoder& owner,
				const class StormByte::Multimedia::Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Encodes one video frame. Opens lazily on first call.
			 * @param owner Public encoder.
			 * @param frame Decoded video frame.
			 * @return true if libav accepted it.
			 */
			bool Push(class StormByte::Multimedia::Pipeline::Encoder& owner,
				const std::shared_ptr<StormByte::Multimedia::Pipeline::Frame>& frame) noexcept override;

			/**
			 * @brief Signals EOF and drains. No-op if already flushed.
			 * @param owner Public encoder.
			 */
			void Flush(class StormByte::Multimedia::Pipeline::Encoder& owner) noexcept override;

			/**
			 * @brief Receives one packet from libav if needed, then pops the queue.
			 * @return Packet with @ref Producer::Encoder, or empty if none ready.
			 */
			std::shared_ptr<StormByte::Multimedia::Pipeline::Packet> Take() noexcept override;

			/**
			 * @brief Opened AVCodecContext, if any.
			 * @return Context, or nullptr before Open().
			 */
			const AVCodecContext* Context() const noexcept override;

			/**
			 * @brief Encoder time base after Open().
			 * @return Rational. {0,1} before Open().
			 */
			AVRational TimeBase() const noexcept override;

		private:
			/**
			 * @brief Receives one backend packet into @ref m_pending.
			 * @param owner Public encoder.
			 * @return true if a packet was queued.
			 */
			bool DrainOne(class StormByte::Multimedia::Pipeline::Encoder& owner) noexcept;

			/**
			 * @brief Shifts pts/dts so the first DTS is 0 and fills duration.
			 */
			void StampOutgoing() noexcept;

			std::optional<StormByte::Multimedia::Backend::FFmpeg::AVEncoder> m_encoder;	///< Opened encoder
			StormByte::Multimedia::Backend::FFmpeg::AVPacket m_scratch;					///< Receive scratch
			std::deque<std::shared_ptr<StormByte::Multimedia::Pipeline::Packet>> m_pending;	///< Packets waiting for Mux
			AVRational m_timeBase;														///< Encoder time base
			int m_index;																///< @ref Encoder::Index after Open
			bool m_flushed;																///< EOF already signalled
			std::int64_t m_tsOffset;													///< Added to pts/dts so the first DTS is 0
			bool m_tsOffsetSet;															///< Offset taken from the first packet
	};
}
