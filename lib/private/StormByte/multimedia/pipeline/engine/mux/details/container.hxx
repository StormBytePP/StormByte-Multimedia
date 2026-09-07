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
#include <StormByte/multimedia/pipeline/engine/mux/engine.hxx>
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
 * @namespace StormByte::Multimedia::Pipeline::Engine::Mux::Details
 * @brief Mux backends.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Mux::Details {
	/**
	 * @class Container
	 * @brief Container mux backend: AVFormatContext, tracks, header-delay queue.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Container final: public Mux::Engine {
		public:
			/**
			 * @class Track
			 * @brief Reserved output slot: encoder remux or bitstream copy.
			 */
			struct Track {
				/**
				 * @brief Live encoder. Null on a copy track.
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

			/**
			 * @brief Empty output context.
			 */
			Container() noexcept;

			/**
			 * @brief Writes trailer if needed and frees the context.
			 */
			~Container() noexcept override;

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
			 * @brief Whether a destination file is bound.
			 * @return true after BindPath().
			 */
			bool IsOpen() const noexcept override;

			/**
			 * @brief Whether avformat_write_header has run.
			 * @return true after a successful header.
			 */
			bool HeaderWritten() const noexcept override;

			/**
			 * @brief Binds the destination path and allocates the format context.
			 * @param owner Public muxer.
			 * @param path Output file.
			 * @return false if owner.Fail() was called.
			 */
			bool BindPath(class Mux& owner, const std::filesystem::path& path) noexcept override;

			/**
			 * @brief Reserves an encode track.
			 * @param owner Public muxer.
			 * @param encoder Live encoder.
			 * @return false if owner.Fail() was called.
			 */
			bool ReserveEncoder(class Mux& owner, class Encoder& encoder) noexcept override;

			/**
			 * @brief Reserves a copy track.
			 * @param owner Public muxer.
			 * @param copy Bound copy handle.
			 * @return false if owner.Fail() was called.
			 */
			bool ReserveCopy(class Mux& owner, const class Copy& copy) noexcept override;

			/**
			 * @brief Snapshots File attachments for header time.
			 * @param owner Public muxer.
			 * @param file Source file.
			 * @return false if owner.Fail() was called.
			 */
			bool BindAttachments(class Mux& owner, const File& file) noexcept override;

			/**
			 * @brief Queues or writes one packet.
			 * @param owner Public muxer.
			 * @param packet Encoded or copied packet.
			 * @return false if owner.Fail() was called.
			 */
			bool Push(class Mux& owner, Packet& packet) noexcept override;

			/**
			 * @brief Flushes reserved encoders, leftover packets and the trailer.
			 * @param owner Public muxer.
			 */
			void Flush(class Mux& owner) noexcept override;

			/**
			 * @brief Writes trailer if needed and frees AVIO + context.
			 */
			void Close() noexcept override;

		private:
			::AVFormatContext* m_ctx = nullptr;								///< Output format context
			std::filesystem::path m_path;									///< Destination path
			std::map<int, Track> m_tracks;									///< Output index → track
			std::map<int, int> m_inToOut;									///< Demux index → output index (copy)
			std::deque<StormByte::Multimedia::Pipeline::Packet> m_queue;	///< Packets waiting for header
			const StormByte::Multimedia::File* m_file = nullptr;			///< Source file (attachments)
			bool m_header = false;											///< avformat_write_header done
			bool m_trailer = false;											///< av_write_trailer done

			/**
			 * @brief Writes header when every reserved encoder is open.
			 * @param owner Public muxer.
			 * @return false if owner.Fail() was called.
			 */
			bool WriteHeaderIfReady(class Mux& owner) noexcept;

			/**
			 * @brief Writes one packet after the header.
			 * @param owner Public muxer.
			 * @param packet Source packet.
			 * @return false if owner.Fail() was called.
			 */
			bool WritePacket(class Mux& owner, Packet& packet) noexcept;
	};
}
