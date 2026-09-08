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
#include <StormByte/multimedia/ocr/engine.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/engine.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>

#include <deque>
#include <optional>

extern "C" {
	#include <libavutil/avutil.h>
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
	 * @class Subtitle
	 * @brief Subtitle encode backend: text/ASS/OCR. No SendFrame.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Subtitle final: public Encoder::Engine {
		public:
			/**
			 * @brief Default constructor.
			 */
			Subtitle() noexcept;

			/**
			 * @brief Destructor.
			 */
			~Subtitle() noexcept override = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Subtitle(const Subtitle&) = delete;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Subtitle& operator=(const Subtitle&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Engine to take.
			 */
			Subtitle(Subtitle&&) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Engine to take.
			 * @return *this.
			 */
			Subtitle& operator=(Subtitle&&) noexcept = default;

			/**
			 * @brief Whether the FFmpeg encoder is open.
			 * @return true after a successful Open().
			 */
			bool IsOpen() const noexcept override;

			/**
			 * @brief Opens the subtitle encoder from the first frame.
			 * @param owner Public encoder.
			 * @param frame First subtitle frame.
			 * @return false if owner.Fail() was called.
			 */
			bool Open(class Encoder& owner, const class Frame& frame) noexcept override;

			/**
			 * @brief Encodes one subtitle cue. Opens lazily on first call.
			 * @param owner Public encoder.
			 * @param frame Decoded subtitle frame.
			 * @return false if owner.Fail() was called.
			 */
			bool Push(class Encoder& owner, class Frame& frame) noexcept override;

			/**
			 * @brief Subtitles do not drain from ReceivePacket.
			 * @param owner Public encoder.
			 * @return Always false.
			 */
			bool DrainOne(class Encoder& owner) noexcept override;

			/**
			 * @brief Marks flushed. Pending packets stay until TakePacket.
			 * @param owner Public encoder.
			 */
			void Flush(class Encoder& owner) noexcept override;

			/**
			 * @brief Pops one pending packet.
			 * @param packet Replaced on success.
			 * @return true if @p packet was filled.
			 */
			bool TakePacket(class Packet& packet) noexcept override;

			/**
			 * @brief Opened AVCodecContext, if any.
			 * @return Context, or nullptr before Open().
			 */
			const AVCodecContext* Context() const noexcept override;

			/**
			 * @brief Encoder time base after Open().
			 * @return Rational. {1, AV_TIME_BASE} before a successful Open().
			 */
			AVRational TimeBase() const noexcept override;

		private:
			std::optional<StormByte::Multimedia::Backend::FFmpeg::AVEncoder> m_encoder;	///< Opened encoder
			StormByte::Multimedia::Backend::FFmpeg::AVPacket m_scratch;					///< Encode scratch
			std::deque<class Packet> m_pending;											///< Packets waiting for Mux
			AVRational m_timeBase{1, AV_TIME_BASE};										///< Encoder time base
			StormByte::Multimedia::OCR::Engine m_ocr;									///< Bitmap OCR
			bool m_flushed = false;														///< Flush already called
	};
}
