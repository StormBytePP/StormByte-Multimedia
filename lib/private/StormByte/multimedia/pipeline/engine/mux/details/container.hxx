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
#include <memory>
#include <optional>
#include <string>

extern "C" {
	#include <libavcodec/codec_par.h>
	#include <libavformat/avformat.h>
	#include <libavutil/rational.h>
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
			 * @brief Reserved output slot for an encoder or a remux track.
			 */
			struct Track {
				/**
				 * @brief Live encoder. Null on a remux slot.
				 */
				StormByte::Multimedia::Pipeline::Encoder* encoder = nullptr;

				/**
				 * @brief Source stream index when this slot is remux. -1 if recode.
				 */
				int inIndex = -1;

				/**
				 * @brief Cloned source codecpar for remux. Owned.
				 */
				::AVCodecParameters* params = nullptr;

				/**
				 * @brief Source time base for remux.
				 */
				AVRational srcTb{0, 1};

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
			 * @brief Copy constructor.
			 * @param other Source engine.
			 */
			Container(const Container& other) = delete;

			/**
			 * @brief Copy assignment.
			 * @param other Source engine.
			 * @return *this.
			 */
			Container& operator=(const Container& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Engine to take.
			 */
			Container(Container&& other) noexcept = delete;

			/**
			 * @brief Move assignment.
			 * @param other Engine to take.
			 * @return *this.
			 */
			Container& operator=(Container&& other) noexcept = delete;

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
			bool BindPath(class StormByte::Multimedia::Pipeline::Muxer& owner,
				const std::filesystem::path& path) noexcept override;

			/**
			 * @brief Reserves an encode track.
			 * @param owner Public muxer.
			 * @param encoder Live encoder.
			 * @return false if owner.Fail() was called.
			 */
			bool ReserveEncoder(class StormByte::Multimedia::Pipeline::Muxer& owner,
				class StormByte::Multimedia::Pipeline::Encoder& encoder) noexcept override;

			/**
			 * @brief Reserves a remux track from @p remux.
			 * @param owner Public muxer.
			 * @param remux Live remuxer.
			 * @return false if owner.Fail() was called.
			 */
			bool ReserveRemux(class StormByte::Multimedia::Pipeline::Muxer& owner,
				class StormByte::Multimedia::Pipeline::Remuxer& remux) noexcept override;

			/**
			 * @brief Snapshots File attachments for header time.
			 * @param owner Public muxer.
			 * @param file Source file.
			 * @return false if owner.Fail() was called.
			 */
			bool BindAttachments(class StormByte::Multimedia::Pipeline::Muxer& owner,
				const File& file) noexcept override;

			/**
			 * @brief Queues or writes one packet.
			 * @param owner Public muxer.
			 * @param packet Encoded or remuxed packet.
			 * @return true if the packet was accepted.
			 */
			bool Push(class StormByte::Multimedia::Pipeline::Muxer& owner,
				const std::shared_ptr<StormByte::Multimedia::Pipeline::Packet>& packet) noexcept override;

			/**
			 * @brief Writes leftover packets and the trailer.
			 * @param owner Public muxer.
			 */
			void Flush(class StormByte::Multimedia::Pipeline::Muxer& owner) noexcept override;

			/**
			 * @brief Writes trailer if needed and frees AVIO + context.
			 */
			void Close() noexcept override;

		private:
			/**
			 * @brief Maps a packet track (source or dest) onto an output slot.
			 * @param track Packet::Track().
			 * @return Output key, or -1.
			 */
			int Resolve(int track) const noexcept;

			/**
			 * @brief Writes header when every reserved encoder is open.
			 * @param owner Public muxer.
			 * @return false if owner.Fail() was called.
			 */
			bool WriteHeaderIfReady(class StormByte::Multimedia::Pipeline::Muxer& owner) noexcept;

			/**
			 * @brief Writes one packet after the header.
			 * @param owner Public muxer.
			 * @param packet Source packet.
			 * @return false if owner.Fail() was called.
			 */
			bool WritePacket(class StormByte::Multimedia::Pipeline::Muxer& owner,
				class StormByte::Multimedia::Pipeline::Packet& packet) noexcept;

			/**
			 * @brief Frees cloned remux codecpar.
			 */
			void FreeParams() noexcept;

			::AVFormatContext* m_ctx;									///< Output format context
			std::filesystem::path m_path;								///< Destination path
			std::map<int, Track> m_tracks;								///< Output index → track
			std::map<int, int> m_inToOut;								///< Source index → output index
			std::deque<std::shared_ptr<StormByte::Multimedia::Pipeline::Packet>> m_queue;	///< Packets waiting for header
			const StormByte::Multimedia::File* m_file;					///< Source file (attachments)
			bool m_header;												///< avformat_write_header done
			bool m_trailer;												///< av_write_trailer done
	};
}
