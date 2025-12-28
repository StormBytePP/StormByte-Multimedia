#pragma once

#include <StormByte/multimedia/exception.hxx>

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	/**
	 * @class DecoderError
	 * @brief The exception for when a decoder error occurs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC DecoderError: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The message of the exception.
			 */
			template <typename... Args>
			DecoderError(std::format_string<Args...> fmt, Args&&... args):
			Exception("Decoder: ", fmt, std::forward<Args>(args)...) {}

			using Exception::Exception;
	};
}