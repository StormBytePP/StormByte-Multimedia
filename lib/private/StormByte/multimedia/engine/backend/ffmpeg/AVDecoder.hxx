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
	 * @class AVDecoder
	 * @brief RAII decoder context with optional BSF pipeline.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVDecoder: public AVPointer<::AVCodecContext> {
		public:
			/**
			 * Copy constructor (deleted).
			 */
			AVDecoder(const AVDecoder&) = delete;

			/**
			 * Move constructor.
			 */
			AVDecoder(AVDecoder&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVDecoder() noexcept override;

			/**
			 * Copy assignment (deleted).
			 */
			AVDecoder& operator=(const AVDecoder&) = delete;

			/**
			 * Move assignment.
			 */
			AVDecoder& operator=(AVDecoder&& other) noexcept = default;

			/**
			 * Opens a decoder from codec + parameters; may attach mp4→Annex-B BSF.
			 * @param codec Decoder codec.
			 * @param params Stream codec parameters.
			 * @param fmt Parent format context (for BSF decision).
			 * @param stream_index Stream index this decoder serves.
			 * @return Decoder or DecoderError.
			 */
			static ExpectedAVDecoder Open(AVCodec* codec, const AVCodecParameters& params, const AVFormatContext& fmt, int stream_index) noexcept;

			/**
			 * Sends a packet (after BSF) to the decoder.
			 * @param pkt Packet for this stream.
			 * @return Operation result.
			 */
			FFmpeg::OperationResult SendPacket(AVPacket& pkt) noexcept;

			/**
			 * Receives a decoded frame.
			 * @param frame Destination frame.
			 * @return Operation result.
			 */
			FFmpeg::OperationResult ReceiveFrame(AVFrame& frame) noexcept;

			/**
			 * @return Stream index this decoder was opened for.
			 */
			int StreamIndex() const noexcept;

			/**
			 * Flushes decoder and BSF buffers.
			 */
			void Flush() noexcept;

			/**
			 * Signals EOF to decoder and BSF.
			 */
			void SetEof() noexcept;

		private:
			int m_stream_index = -1;			///< Bound stream index
			AVBSFPipeline m_bsf_pipeline;	///< Optional BSF chain

			/**
			 * @param ctx Opened codec context.
			 */
			explicit AVDecoder(AVCodecContext* ctx) noexcept;

			/**
			 * Frees the codec context.
			 */
			void Free() noexcept override;
	};
}
