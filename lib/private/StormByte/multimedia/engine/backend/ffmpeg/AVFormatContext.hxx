/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia is dual-licensed under the following terms:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You can redistribute it and/or modify it under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this software may be used under the terms of a
 *    commercial license agreement with the sole copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *    Contact the copyright holder for more information.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this StormByte-Multimedia. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/typedefs.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>
#include <optional>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
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
			 * Copy constructor (deleted).
			 */
			AVFormatContext(const AVFormatContext& other) = delete;

			/**
			 * Move constructor.
			 */
			AVFormatContext(AVFormatContext&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVFormatContext() noexcept override;

			/**
			 * Copy assignment (deleted).
			 */
			AVFormatContext& operator=(const AVFormatContext& other) = delete;

			/**
			 * Move assignment.
			 */
			AVFormatContext& operator=(AVFormatContext&& other) noexcept = default;

			/**
			 * Opens a file and finds stream info.
			 * @param path Media path.
			 * @return Context or DecoderError.
			 */
			static ExpectedAVFormatContext Open(const std::filesystem::path& path);

			/**
			 * @return Container metadata dictionary as Metadata.
			 */
			StormByte::Multimedia::Metadata Metadata() const noexcept;

			/**
			 * Reads the next packet.
			 * @param packet Destination packet.
			 * @return Operation result.
			 */
			OperationResult ReadPacket(AVPacket& packet) noexcept;

			/**
			 * @return Set of non-owning stream views.
			 */
			Streams Streams() const noexcept;

			/**
			 * Returns an mp4→Annex-B BSF when the container and codec require it.
			 * @param codec_id Codec id.
			 * @param stream_id Stream index (time base).
			 * @param params Codec parameters.
			 * @return BSF or nullopt.
			 */
			std::optional<AVBSF> Mp4ToAnnexB(int codec_id, int stream_id, const AVCodecParameters& params) const noexcept;

		private:
			/**
			 * @param ctx Raw format context (owned).
			 */
			explicit AVFormatContext(::AVFormatContext* ctx) noexcept;

			/**
			 * Closes input (avformat_close_input).
			 */
			void Free() noexcept override;
	};
}
