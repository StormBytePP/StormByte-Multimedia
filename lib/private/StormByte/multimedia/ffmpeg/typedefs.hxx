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
	class AVDecoder;
	class AVFormatContext;
	class AVStream;
	struct StreamLess;
	enum OperationResult {
		Success,
		EndOfFile,
		Error,
		TryAgain,
	};
	using ExpectedAVFormatContext = StormByte::Expected<AVFormatContext, FFmpeg::DecoderError>;
	using ExpectedAVDecoder	= StormByte::Expected<AVDecoder, FFmpeg::DecoderError>;
	using Streams = std::set<AVStream, StreamLess>;

	inline std::string ErrorToString(int errnum) {
		char buf[AV_ERROR_MAX_STRING_SIZE] = {0};
		av_strerror(errnum, buf, sizeof(buf));
		return buf;
	}
}