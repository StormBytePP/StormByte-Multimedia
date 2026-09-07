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

#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Decoder
 * @brief Private decode backends selected by stream type.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Decoder {
	/**
	 * @class Engine
	 * @brief Abstract decode backend. One instance per public Decoder.
	 *
	 * The public Decoder is the entry point. Open, Send, Receive and Flush live here.
	 * Details::Video / Details::Audio / Details::Subtitle implement this.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Engine {
		public:
			/**
			 * @brief Destructor.
			 */
			virtual ~Engine() noexcept = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Engine(const Engine&) = delete;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Engine& operator=(const Engine&) = delete;

			/**
			 * @brief Whether the FFmpeg decoder is open.
			 * @return true after a successful Open().
			 */
			virtual bool IsOpen() const noexcept = 0;

			/**
			 * @brief Sends one compressed packet. Ignores other stream indexes.
			 * @param owner Public decoder (Fail, Index, tags).
			 * @param packet Compressed packet.
			 * @return false if owner.Fail() was called.
			 */
			virtual bool Send(class Decoder& owner, Packet& packet) noexcept = 0;

			/**
			 * @brief Receives one decoded frame. No-op on TryAgain.
			 * @param owner Public decoder.
			 * @param frame Replaced on success.
			 * @return true if @p frame was filled.
			 */
			virtual bool Receive(class Decoder& owner, class Frame& frame) noexcept = 0;

			/**
			 * @brief Signals EOF to libavcodec. Drain with Receive afterwards.
			 * @param owner Public decoder.
			 */
			virtual void Flush(class Decoder& owner) noexcept = 0;

		protected:
			/**
			 * @brief Default constructor.
			 */
			Engine() noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Engine to take.
			 */
			Engine(Engine&&) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Engine to take.
			 * @return *this.
			 */
			Engine& operator=(Engine&&) noexcept = default;
	};
}
