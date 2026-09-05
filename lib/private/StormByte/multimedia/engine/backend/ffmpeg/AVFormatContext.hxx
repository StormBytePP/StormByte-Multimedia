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

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/typedefs.hxx>
// oldcode — Metadata se rehace
// #include <StormByte/multimedia/metadata.hxx>

#include <filesystem>
#include <optional>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVBSF;
	class AVCodecParameters;
	class AVPacket;
	class AVStream;

	/**
	 * @class AVFormatContext
	 * @brief RAII input format context (demuxer).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVFormatContext: public AVPointer<::AVFormatContext> {
		public:
			/**
			 * @brief Copy constructor (deleted).
			 * @param other Unused.
			 */
			AVFormatContext(const AVFormatContext& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source context.
			 */
			AVFormatContext(AVFormatContext&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~AVFormatContext() noexcept override;

			/**
			 * @brief Copy assignment (deleted).
			 * @param other Unused.
			 * @return *this.
			 */
			AVFormatContext& operator=(const AVFormatContext& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source context.
			 * @return *this.
			 */
			AVFormatContext& operator=(AVFormatContext&& other) noexcept = default;

			/**
			 * @brief Opens a file and finds stream info.
			 * @param path Media path.
			 * @return Context or DecoderError.
			 */
			static ExpectedAVFormatContext Open(const std::filesystem::path& path);

			// oldcode — API pública Metadata
			// /**
			//  * @return Container metadata dictionary as Metadata.
			//  */
			// StormByte::Multimedia::Metadata Metadata() const noexcept;

			/**
			 * @brief Reads the next packet.
			 * @param packet Destination packet.
			 * @return Operation result.
			 */
			OperationResult ReadPacket(AVPacket& packet) noexcept;

			/**
			 * @brief Non-owning stream views.
			 * @return Set of streams.
			 */
			Streams Streams() const noexcept;

			/**
			 * @brief Returns an mp4→Annex-B BSF when the container and codec require it.
			 * @param codec_id Codec id.
			 * @param stream_id Stream index (time base).
			 * @param params Codec parameters.
			 * @return BSF or nullopt.
			 */
			std::optional<AVBSF> Mp4ToAnnexB(int codec_id, int stream_id, const AVCodecParameters& params) const noexcept;

		private:
			/**
			 * @brief Adopts a raw format context.
			 * @param ctx Raw format context (owned).
			 */
			explicit AVFormatContext(::AVFormatContext* ctx) noexcept;

			/**
			 * @brief Closes input (avformat_close_input).
			 */
			void Free() noexcept override;
	};
}
