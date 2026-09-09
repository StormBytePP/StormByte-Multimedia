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
#include <StormByte/multimedia/backend/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/engine.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/property/audio.hxx>
#include <StormByte/multimedia/property/duration.hxx>

#include <memory>
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
	 * @class Audio
	 * @brief Audio decode backend. Sample-format quirks belong here later.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Audio final: public Decoder::Engine {
		public:
			/**
			 * @brief Adopts an opened audio decoder.
			 * @param decoder Opened AVDecoder.
			 * @param timeBase Stream time base.
			 * @param audio Stream audio properties.
			 */
			Audio(StormByte::Multimedia::Backend::FFmpeg::AVDecoder decoder,
				AVRational timeBase,
				std::optional<StormByte::Multimedia::Property::Audio> audio) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Audio() noexcept override = default;

			/**
			 * @brief Copy constructor.
			 * @param other Source engine.
			 */
			Audio(const Audio& other) = delete;

			/**
			 * @brief Copy assignment.
			 * @param other Source engine.
			 * @return *this.
			 */
			Audio& operator=(const Audio& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Engine to take.
			 */
			Audio(Audio&& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Engine to take.
			 * @return *this.
			 */
			Audio& operator=(Audio&& other) noexcept = default;

			/**
			 * @brief Whether the FFmpeg decoder is open.
			 * @return true.
			 */
			bool IsOpen() const noexcept override;

			/**
			 * @brief Sends one audio packet.
			 * @param owner Public decoder.
			 * @param packet Compressed packet.
			 * @return true if libav accepted it.
			 */
			bool Send(class StormByte::Multimedia::Pipeline::Decoder& owner,
				const std::shared_ptr<StormByte::Multimedia::Pipeline::Packet>& packet) noexcept override;

			/**
			 * @brief Receives one audio frame.
			 * @param owner Public decoder.
			 * @return Frame with @ref Producer::Decoder, or empty if none ready.
			 */
			std::shared_ptr<StormByte::Multimedia::Pipeline::Frame> Receive(
				class StormByte::Multimedia::Pipeline::Decoder& owner) noexcept override;

			/**
			 * @brief Signals EOF and keeps delayed frames available to Receive.
			 * @param owner Public decoder.
			 */
			void Flush(class StormByte::Multimedia::Pipeline::Decoder& owner) noexcept override;

		private:
			StormByte::Multimedia::Backend::FFmpeg::AVDecoder m_decoder;					///< Opened decoder
			StormByte::Multimedia::Backend::FFmpeg::AVFrame m_scratch;						///< Receive scratch
			std::optional<StormByte::Multimedia::Property::Audio> m_audio;					///< Stream audio properties
			AVRational m_timeBase;															///< Stream time base
			bool m_flushed;																	///< EOF already signalled
	};
}
