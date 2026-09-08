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

#include <StormByte/multimedia/backend/ffmpeg/AVDecoder.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVSubtitle.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/engine.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/property/duration.hxx>

#include <optional>

extern "C" {
	#include <libavutil/rational.h>
}

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Decoder::Details
 * @brief Per-media decode engines.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Decoder::Details {
	/**
	 * @class Subtitle
	 * @brief Subtitle decode backend: text/ASS/bitmap OCR payload. No AVFrame.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Subtitle final: public Decoder::Engine {
		public:
			/**
			 * @brief Adopts an opened subtitle decoder.
			 * @param decoder Opened AVDecoder.
			 * @param timeBase Stream time base.
			 */
			Subtitle(StormByte::Multimedia::Backend::FFmpeg::AVDecoder decoder,
				AVRational timeBase) noexcept;

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
			 * @brief Whether the FFmpeg decoder is open.
			 * @return true.
			 */
			bool IsOpen() const noexcept override;

			/**
			 * @brief Sends one subtitle packet.
			 * @param owner Public decoder.
			 * @param packet Compressed packet.
			 * @return false if owner.Fail() was called.
			 */
			bool Send(class Decoder& owner, class Packet& packet) noexcept override;

			/**
			 * @brief Receives one subtitle frame (text or OCR-prefixed bitmap).
			 * @param owner Public decoder.
			 * @param frame Replaced on success.
			 * @return true if @p frame was filled.
			 */
			bool Receive(class Decoder& owner, class Frame& frame) noexcept override;

			/**
			 * @brief Signals EOF. Held cues stay available to Receive.
			 * @param owner Public decoder.
			 */
			void Flush(class Decoder& owner) noexcept override;

		private:
			StormByte::Multimedia::Backend::FFmpeg::AVDecoder m_decoder;					///< Opened decoder
			std::optional<StormByte::Multimedia::Backend::FFmpeg::AVSubtitle> m_pendingSub;	///< Pending AVSubtitle
			std::optional<StormByte::Multimedia::Pipeline::Frame> m_heldSubtitle;			///< Held subtitle frame
			std::optional<StormByte::Multimedia::Property::Duration> m_packetPts;			///< Last packet PTS
			std::optional<StormByte::Multimedia::Property::Duration> m_packetDuration;		///< Last packet duration
			AVRational m_timeBase{0, 1};													///< Stream time base
			bool m_flushed = false;															///< EOF already signalled
	};
}
