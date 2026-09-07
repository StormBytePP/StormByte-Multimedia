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

#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>

#include <cstdint>
#include <deque>
#include <filesystem>
#include <map>
#include <optional>
#include <string>

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavutil/rational.h>
}

namespace StormByte::Multimedia::Pipeline {
	class Copy;
}

/**
 * @class StormByte::Multimedia::Pipeline::Mux::Impl
 * @brief FFmpeg output context, reserved tracks and header-delay queue.
 */
class StormByte::Multimedia::Pipeline::Mux::Impl {
	public:
		/**
		 * @brief Empty output context.
		 */
		Impl() noexcept = default;

		/**
		 * @brief Writes trailer if needed and frees the context.
		 */
		~Impl() noexcept {
			Close();
		}

		/**
		 * @brief Copy constructor (deleted).
		 */
		Impl(const Impl&) = delete;

		/**
		 * @brief Copy assignment (deleted).
		 * @return *this.
		 */
		Impl& operator=(const Impl&) = delete;

		/**
		 * @class Track
		 * @brief Reserved output slot: encoder remux or bitstream copy.
		 */
		struct Track {
			/**
			 * @brief Live encoder. Null on a copy track. Non-const so Flush() can drain it.
			 */
			StormByte::Multimedia::Pipeline::Encoder* encoder = nullptr;

			/**
			 * @brief Live copy handle. Null on an encoder track.
			 */
			const StormByte::Multimedia::Pipeline::Copy* copy = nullptr;

			/**
			 * @brief Demux stream index for a copy track. -1 on encode.
			 */
			int inIndex = -1;

			/**
			 * @brief Index in AVFormatContext after the header.
			 */
			int avIndex = -1;

			/**
			 * @brief Time base used to rescale packet timestamps.
			 */
			AVRational timeBase{0, 1};

			/**
			 * @brief Last written DTS in @ref timeBase ticks.
			 */
			std::int64_t lastDts = AV_NOPTS_VALUE;

			/**
			 * @brief Stream language tag to write on the header.
			 */
			std::optional<std::string> language;

			/**
			 * @brief Stream title tag to write on the header.
			 */
			std::optional<std::string> title;
		};

		::AVFormatContext* m_ctx = nullptr;								///< Output format context
		std::filesystem::path m_path;									///< Destination path
		std::map<int, Track> m_tracks;									///< Output index → track
		std::map<int, int> m_inToOut;									///< Demux index → output index (copy)
		std::deque<StormByte::Multimedia::Pipeline::Packet> m_queue;	///< Packets waiting for header
		const StormByte::Multimedia::File* m_file = nullptr;			///< Source file (attachments)
		bool m_header = false;											///< avformat_write_header done
		bool m_trailer = false;											///< av_write_trailer done

		/**
		 * @brief Writes trailer and frees AVIO + context.
		 */
		void Close() noexcept {
			if (!m_ctx)
				return;
			if (m_header && !m_trailer) {
				av_write_trailer(m_ctx);
				m_trailer = true;
			}
			if (m_ctx->pb && !(m_ctx->oformat && (m_ctx->oformat->flags & AVFMT_NOFILE)))
				avio_closep(&m_ctx->pb);
			avformat_free_context(m_ctx);
			m_ctx = nullptr;
		}
};
