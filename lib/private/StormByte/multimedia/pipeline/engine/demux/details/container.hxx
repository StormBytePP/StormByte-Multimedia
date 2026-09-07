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

#include <StormByte/multimedia/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/engine/demux/engine.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>

#include <optional>
#include <unordered_map>

extern "C" {
	#include <libavutil/rational.h>
}

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Demux::Details
 * @brief Demux backends.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Demux::Details {
	/**
	 * @class Container
	 * @brief Container demux backend: AVFormatContext, time bases, packet scratch.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Container final: public Demux::Engine {
		public:
			/**
			 * @brief Empty backend (not open).
			 */
			Container() noexcept;

			/**
			 * @brief Destructor.
			 */
			~Container() noexcept override = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Container(const Container&) = delete;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Container& operator=(const Container&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Engine to take.
			 */
			Container(Container&&) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Engine to take.
			 * @return *this.
			 */
			Container& operator=(Container&&) noexcept = default;

			/**
			 * @brief Whether the format context is open.
			 * @return true after a successful Open().
			 */
			bool IsOpen() const noexcept override;

			/**
			 * @brief Adopts an already opened format context.
			 * @param owner Public demuxer.
			 * @param file Unused. Signature matches Engine::Open; the context is passed via Adopt().
			 * @return false. Use Adopt() from file >> demux.
			 */
			bool Open(class Demux& owner, const File& file) noexcept override;

			/**
			 * @brief Adopts an opened format context and caches time bases.
			 * @param owner Public demuxer.
			 * @param ctx Opened AVFormatContext.
			 * @return false if owner.Fail() was called.
			 */
			bool Adopt(class Demux& owner, StormByte::Multimedia::Backend::FFmpeg::AVFormatContext ctx) noexcept;

			/**
			 * @brief Reads one compressed packet.
			 * @param owner Public demuxer.
			 * @param packet Replaced on success.
			 * @return true if @p packet was filled.
			 */
			bool Read(class Demux& owner, Packet& packet) noexcept override;

			/**
			 * @brief Raw AVFormatContext pointer.
			 * @return Context pointer, or nullptr.
			 */
			void* Context() noexcept override;

			/**
			 * @brief Opened AVFormatContext (typed).
			 * @return Context, or nullptr.
			 */
			StormByte::Multimedia::Backend::FFmpeg::AVFormatContext* Format() noexcept;

			/**
			 * @brief Stream time base cache.
			 * @return Index → time base.
			 */
			const std::unordered_map<int, AVRational>& TimeBases() const noexcept;

		private:
			std::optional<StormByte::Multimedia::Backend::FFmpeg::AVFormatContext> m_ctx;	///< Opened source
			std::unordered_map<int, AVRational> m_timeBase;									///< Stream index → time base
			StormByte::Multimedia::Backend::FFmpeg::AVPacket m_scratch;						///< Read scratch
	};
}
