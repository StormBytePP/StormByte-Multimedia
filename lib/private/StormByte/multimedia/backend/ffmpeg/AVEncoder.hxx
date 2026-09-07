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

#include <StormByte/multimedia/backend/ffmpeg/AVBSFPipeline.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/backend/ffmpeg/typedefs.hxx>

#include <map>
#include <string>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libavutil/rational.h>
}

/**
 * @namespace StormByte::Multimedia::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Backend::FFmpeg {
	class AVCodecParameters;
	class AVFormatContext;
	class AVFrame;
	class AVPacket;

	/**
	 * @class AVEncoder
	 * @brief RAII encoder context with optional BSF pipeline.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVEncoder: public AVPointer<::AVCodecContext> {
		public:
			/**
			 * @brief Copy constructor (deleted).
			 */
			AVEncoder(const AVEncoder&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source encoder.
			 */
			AVEncoder(AVEncoder&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~AVEncoder() noexcept override;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			AVEncoder& operator=(const AVEncoder&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source encoder.
			 * @return *this.
			 */
			AVEncoder& operator=(AVEncoder&& other) noexcept = default;

			/**
			 * @brief Opens an encoder; may attach BSF from @p fmt.
			 * @param codec Encoder codec.
			 * @param params Stream codec parameters.
			 * @param fmt Format context (for BSF decision).
			 * @param stream_index Stream index.
			 * @return Encoder or EncoderError.
			 */
			static ExpectedAVEncoder Open(AVCodec* codec, const AVCodecParameters& params, const AVFormatContext& fmt, int stream_index) noexcept;

			/**
			 * @brief Opens an encoder without a muxer (no BSF).
			 * @param codec Encoder codec.
			 * @param params Stream codec parameters.
			 * @param stream_index Output track index.
			 * @param options av_opt_set pairs applied before avcodec_open2.
			 * @param time_base Encoder time base. Ignored if num/den are not positive.
			 * @return Encoder or EncoderError.
			 */
			static ExpectedAVEncoder Open(AVCodec* codec, const AVCodecParameters& params, int stream_index,
				const std::map<std::string, std::string>& options, AVRational time_base) noexcept;

			/**
			 * @brief Sends a frame to the encoder.
			 * @param frame Source frame.
			 * @return Operation result.
			 */
			FFmpeg::OperationResult SendFrame(AVFrame& frame) noexcept;

			/**
			 * @brief Receives an encoded packet (after BSF).
			 * @param pkt Destination packet.
			 * @return Operation result.
			 */
			FFmpeg::OperationResult ReceivePacket(AVPacket& pkt) noexcept;

			/**
			 * @brief Stream index this encoder was opened for.
			 * @return Stream index.
			 */
			int StreamIndex() const noexcept;

			/**
			 * @brief Encoder time base.
			 * @return `time_base`, or 0/1.
			 */
			AVRational TimeBase() const noexcept;

			/**
			 * @brief Flushes encoder and BSF.
			 */
			void Flush() noexcept;

			/**
			 * @brief Signals EOF to encoder and BSF.
			 */
			void SetEof() noexcept;

		private:
			int m_stream_index = -1;
			FFmpeg::AVBSFPipeline m_bsf_pipeline;

			/**
			 * @brief Adopts an opened codec context.
			 * @param ctx Opened codec context.
			 */
			explicit AVEncoder(AVCodecContext* ctx) noexcept;

			/**
			 * @brief Frees the codec context.
			 */
			void Free() noexcept override;
	};
}
