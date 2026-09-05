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

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavcodec/bsf.h>
}

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
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
			 * @brief Copy constructor (deleted).
			 */
			AVBSF(const AVBSF&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source filter.
			 */
			AVBSF(AVBSF&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~AVBSF() noexcept override;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			AVBSF& operator=(const AVBSF&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source filter.
			 * @return *this.
			 */
			AVBSF& operator=(AVBSF&& other) noexcept;

			/**
			 * @brief Creates and initializes a named BSF.
			 * @param name Filter name (e.g. "h264_mp4toannexb").
			 * @param params Input codec parameters.
			 * @param time_base Input time base.
			 * @return AVBSF or BSFError.
			 */
			static ExpectedAVBSF Create(const std::string& name, const AVCodecParameters& params, AVRational time_base) noexcept;

			/**
			 * @brief Sends a packet into the filter.
			 * @param pkt Packet to send.
			 * @return Operation result.
			 */
			OperationResult SendPacket(AVPacket& pkt) noexcept;

			/**
			 * @brief Receives a filtered packet.
			 * @param pkt Destination packet.
			 * @return Operation result.
			 */
			OperationResult ReceivePacket(AVPacket& pkt) noexcept;

			/**
			 * @brief Flushes the filter.
			 */
			void Flush() noexcept;

			/**
			 * @brief Signals EOF (null packet).
			 */
			void SetEof() noexcept;

		private:
			/**
			 * @brief Adopts an allocated BSF context.
			 * @param ctx Allocated BSF context.
			 */
			explicit AVBSF(AVBSFContext* ctx) noexcept;

			/**
			 * @brief Frees the BSF (av_bsf_free).
			 */
			void Free() noexcept override;
	};
}
