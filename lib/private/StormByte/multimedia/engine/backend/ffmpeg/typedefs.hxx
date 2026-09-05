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

#include <StormByte/expected.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/exception.hxx>

#include <set>

extern "C" {
	#include <libavutil/error.h>
}

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @enum OperationResult
	 * @brief Result of send/receive style FFmpeg calls.
	 */
	enum STORMBYTE_MULTIMEDIA_PRIVATE OperationResult {
		Success,	///< Completed successfully
		EndOfFile,	///< EOF reached
		Error,		///< Hard error
		TryAgain	///< EAGAIN — need more input/output
	};

	class AVBSF;
	class AVDecoder;
	class AVEncoder;
	class AVFormatContext;
	class AVStream;

	using ExpectedAVFormatContext = StormByte::Expected<AVFormatContext, FFmpeg::DecoderError>;	///< Open demuxer
	using ExpectedAVDecoder = StormByte::Expected<AVDecoder, FFmpeg::DecoderError>;			///< Open decoder
	using ExpectedAVEncoder = StormByte::Expected<AVEncoder, FFmpeg::EncoderError>;			///< Open encoder
	using ExpectedAVBSF = StormByte::Expected<AVBSF, FFmpeg::BSFError>;				///< Create BSF
	using Streams = std::set<AVStream>;								///< Stream set

	/**
	 * @brief Converts an FFmpeg error code to a string.
	 * @param errnum Code from av_strerror.
	 * @return Human-readable message.
	 */
	inline std::string ErrorToString(int errnum) {
		char buf[AV_ERROR_MAX_STRING_SIZE] = {0};
		av_strerror(errnum, buf, sizeof(buf));
		return buf;
	}
}
