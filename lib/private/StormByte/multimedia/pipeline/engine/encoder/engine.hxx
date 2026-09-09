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

#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>

extern "C" {
	struct AVCodecContext;
	#include <libavutil/rational.h>
}

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Encoder
 * @brief Private encode backends selected by Codec::Type().
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Encoder {
	/**
	 * @class Engine
	 * @brief Abstract encode backend. One instance per public Encoder.
	 *
	 * Push / Take use shared_ptr. Errors are owner.Fail.
	 * Take empty means no packet ready (or drain finished after Flush).
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
			 * @brief Copy constructor.
			 * @param other Source engine.
			 */
			Engine(const Engine& other) = delete;

			/**
			 * @brief Copy assignment.
			 * @param other Source engine.
			 * @return *this.
			 */
			Engine& operator=(const Engine& other) = delete;

			/**
			 * @brief Whether the FFmpeg encoder is open.
			 * @return true after a successful Open().
			 */
			virtual bool IsOpen() const noexcept = 0;

			/**
			 * @brief Picks the table row and opens the FFmpeg encoder from @p frame.
			 * @param owner Public encoder (setters, Fail, tags).
			 * @param frame First frame (resolution, HDR, layout, sample fmt).
			 * @return false if owner.Fail() was called.
			 */
			virtual bool Open(class StormByte::Multimedia::Pipeline::Encoder& owner,
				const class StormByte::Multimedia::Pipeline::Frame& frame) noexcept = 0;

			/**
			 * @brief Encodes one frame. Opens lazily on first call if needed.
			 * @param owner Public encoder.
			 * @param frame Decoded frame.
			 * @return true if libav accepted it.
			 */
			virtual bool Push(class StormByte::Multimedia::Pipeline::Encoder& owner,
				const std::shared_ptr<StormByte::Multimedia::Pipeline::Frame>& frame) noexcept = 0;

			/**
			 * @brief Signals EOF and drains. No-op if already flushed.
			 * @param owner Public encoder.
			 */
			virtual void Flush(class StormByte::Multimedia::Pipeline::Encoder& owner) noexcept = 0;

			/**
			 * @brief Pops one pending encoded packet.
			 * @return Packet with @ref Producer::Encoder, or empty if none ready.
			 */
			virtual std::shared_ptr<StormByte::Multimedia::Pipeline::Packet> Take() noexcept = 0;

			/**
			 * @brief Opened AVCodecContext, if any.
			 * @return Context, or nullptr before Open().
			 */
			virtual const AVCodecContext* Context() const noexcept = 0;

			/**
			 * @brief Encoder time base after Open().
			 * @return Rational. {0,1} before Open().
			 */
			virtual AVRational TimeBase() const noexcept = 0;

		protected:
			/**
			 * @brief Default constructor.
			 */
			Engine() noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Engine to take.
			 */
			Engine(Engine&& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Engine to take.
			 * @return *this.
			 */
			Engine& operator=(Engine&& other) noexcept = default;
	};
}
