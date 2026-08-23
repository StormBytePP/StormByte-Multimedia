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

#include <StormByte/multimedia/engine/backend/ffmpeg/AVBSFPipeline.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/typedefs.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
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
			 * Copy constructor (deleted).
			 */
			AVEncoder(const AVEncoder& other) = delete;

			/**
			 * Move constructor.
			 */
			AVEncoder(AVEncoder&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVEncoder() noexcept override;

			/**
			 * Copy assignment (deleted).
			 */
			AVEncoder& operator=(const AVEncoder&) = delete;

			/**
			 * Move assignment.
			 */
			AVEncoder& operator=(AVEncoder&& other) noexcept;

			/**
			 * Opens an encoder from codec + parameters; may attach BSF.
			 * @param codec Encoder codec.
			 * @param params Stream codec parameters.
			 * @param fmt Format context (for BSF decision).
			 * @param stream_index Stream index.
			 * @return Encoder or EncoderError.
			 */
			static ExpectedAVEncoder Open(AVCodec* codec, const AVCodecParameters& params, const AVFormatContext& fmt, int stream_index) noexcept;

			/**
			 * Sends a frame to the encoder.
			 * @param frame Source frame.
			 * @return Operation result.
			 */
			FFmpeg::OperationResult SendFrame(AVFrame& frame) noexcept;

			/**
			 * Receives an encoded packet (after BSF).
			 * @param pkt Destination packet.
			 * @return Operation result.
			 */
			FFmpeg::OperationResult ReceivePacket(AVPacket& pkt) noexcept;

			/**
			 * @return Stream index this encoder was opened for.
			 */
			int StreamIndex() const noexcept;

			/**
			 * Flushes encoder and BSF.
			 */
			void Flush() noexcept;

			/**
			 * Signals EOF to encoder and BSF.
			 */
			void SetEof() noexcept;

		private:
			int m_stream_index = -1;					///< Bound stream index
			FFmpeg::AVBSFPipeline m_bsf_pipeline;	///< Optional BSF chain

			/**
			 * @param ctx Opened codec context.
			 */
			explicit AVEncoder(AVCodecContext* ctx) noexcept;

			/**
			 * Frees the codec context.
			 */
			void Free() noexcept override;
	};
}
