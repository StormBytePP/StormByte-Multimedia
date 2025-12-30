#pragma once

#include <StormByte/expected.hxx>
#include <StormByte/multimedia/ffmpeg/exception.hxx>

#include <set>

extern "C" {
	#include <libavutil/error.h>
}

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	/**
	 * @enum OperationResult
	 * @brief The result of an operation.
	 */
	enum STORMBYTE_MULTIMEDIA_ADVANCED OperationResult {
		Success,						///< Operation was successful
		EndOfFile,						///< End of file reached
		Error,							///< An error occurred
		TryAgain,						///< Try the operation again
	};

	class AVBSF;
	class AVDecoder;
	class AVEncoder;
	class AVFormatContext;
	class AVStream;

	using ExpectedAVFormatContext = StormByte::Expected<AVFormatContext, FFmpeg::DecoderError>;
	using ExpectedAVDecoder	= StormByte::Expected<AVDecoder, FFmpeg::DecoderError>;
	using ExpectedAVEncoder = StormByte::Expected<AVEncoder, FFmpeg::EncoderError>;
	using ExpectedAVBSF		= StormByte::Expected<AVBSF, FFmpeg::BSFError>;
	using Streams = std::set<AVStream>;

	inline std::string ErrorToString(int errnum) {
		char buf[AV_ERROR_MAX_STRING_SIZE] = {0};
		av_strerror(errnum, buf, sizeof(buf));
		return buf;
	}
}