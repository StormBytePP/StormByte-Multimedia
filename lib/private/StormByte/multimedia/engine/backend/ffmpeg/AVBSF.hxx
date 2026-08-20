/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/typedefs.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavcodec/bsf.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVCodecParameters;
	class AVPacket;

	/**
	 * @class AVBSF
	 * @brief RAII bitstream filter context.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVBSF: public AVPointer<::AVBSFContext> {
		public:
			/**
			 * Copy constructor (deleted).
			 */
			AVBSF(const AVBSF&) = delete;

			/**
			 * Move constructor.
			 */
			AVBSF(AVBSF&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVBSF() noexcept override;

			/**
			 * Copy assignment (deleted).
			 */
			AVBSF& operator=(const AVBSF&) = delete;

			/**
			 * Move assignment.
			 */
			AVBSF& operator=(AVBSF&& other) noexcept;

			/**
			 * Creates and initializes a named BSF.
			 * @param name Filter name (e.g. "h264_mp4toannexb").
			 * @param params Input codec parameters.
			 * @param time_base Input time base.
			 * @return AVBSF or BSFError.
			 */
			static ExpectedAVBSF Create(const std::string& name, const AVCodecParameters& params, AVRational time_base) noexcept;

			/**
			 * Sends a packet into the filter.
			 * @param pkt Packet to send.
			 * @return Operation result.
			 */
			OperationResult SendPacket(AVPacket& pkt) noexcept;

			/**
			 * Receives a filtered packet.
			 * @param pkt Destination packet.
			 * @return Operation result.
			 */
			OperationResult ReceivePacket(AVPacket& pkt) noexcept;

			/**
			 * Flushes the filter.
			 */
			void Flush() noexcept;

			/**
			 * Signals EOF (null packet).
			 */
			void SetEof() noexcept;

		private:
			/**
			 * @param ctx Allocated BSF context.
			 */
			explicit AVBSF(AVBSFContext* ctx) noexcept;

			/**
			 * Frees the BSF (av_bsf_free).
			 */
			void Free() noexcept override;
	};
}
