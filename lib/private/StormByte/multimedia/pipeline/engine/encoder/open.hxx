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
#include <StormByte/multimedia/features.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>

#include <optional>
#include <string>

extern "C" {
	#include <libavutil/rational.h>
}

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Encoder::Open
 * @brief Shared FFmpeg open + packet wrap used by the three encode engines.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Encoder::Open {
	/**
	 * @struct Backend
	 * @brief Result of a successful table lookup and avcodec_open2.
	 */
	struct Backend {
		StormByte::Multimedia::Backend::FFmpeg::AVEncoder encoder;	///< Opened encoder
		AVRational timeBase{0, 1};									///< Encoder time base
		std::string implementation;									///< Selected row name
		StormByte::Multimedia::Features capabilities;				///< Selected row features
	};

	/**
	 * @struct Access
	 * @brief Friend of Encoder and Frame. Calls owner.Fail() and reads Frame backend.
	 */
	struct Access {
		/**
		 * @brief Picks the table row, applies owner setters and opens FFmpeg.
		 * @param owner Public encoder (Fail, CRF, FineTune, Destination).
		 * @param frame First frame.
		 * @return Backend, or empty after owner.Fail().
		 */
		static std::optional<Backend> Open(class Encoder& owner, const class Frame& frame) noexcept;
	};

	/**
	 * @brief Picks the table row, applies owner setters and opens FFmpeg.
	 * @param owner Public encoder (Fail, CRF, FineTune, Destination).
	 * @param frame First frame.
	 * @return Backend, or empty after owner.Fail().
	 */
	inline std::optional<Backend> OpenBackend(class Encoder& owner, const class Frame& frame) noexcept {
		return Access::Open(owner, frame);
	}

	/**
	 * @brief Builds a pipeline Packet from an encoded AVPacket.
	 * @param type Kind of the encoded access unit (destination codec Type).
	 * @param index Mux output index.
	 * @param raw Encoder output.
	 * @param timeBase Encoder time base.
	 * @param keepPacketHdrPlus false for HEVC (SEI already in payload).
	 * @return Move-only packet.
	 */
	class Packet MakePacket(enum Type type, int index,
		const StormByte::Multimedia::Backend::FFmpeg::AVPacket& raw,
		AVRational timeBase, bool keepPacketHdrPlus = true) noexcept;

	/**
	 * @brief Converts nanoseconds to encoder ticks.
	 * @param ns Nanoseconds.
	 * @param timeBase Encoder time base.
	 * @return Ticks, or AV_NOPTS_VALUE.
	 */
	std::int64_t NsToTicks(std::int64_t ns, AVRational timeBase) noexcept;
}
