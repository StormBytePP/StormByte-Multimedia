#pragma once

#include <StormByte/multimedia/exception.hxx>

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::FFmpeg {
	class STORMBYTE_MULTIMEDIA_ADVANCED Exception: public Multimedia::Exception {
		public:
			/**
			 * @brief Constructor forwards the message to the `std::format` function
			 * @tparam Args Format argument types
			 * @param fmt Format string
			 * @param args Arguments for formatting
			 * 
			 * If no arguments are provided, the format string is used as the exception message directly.
			 */
			template <typename... Args>
			Exception(const std::string& component, std::format_string<Args...> fmt, Args&&... args):
			StormByte::Exception("AV::" + component, fmt, std::forward<Args>(args)...) {}

			using Multimedia::Exception::Exception;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Exception() noexcept 								= default;
	};

	/**
	 * @class BSFError
	 * @brief The exception for when a bitstream filter error occurs.
	 */
	class STORMBYTE_MULTIMEDIA_ADVANCED BSFError: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The message of the exception.
			 */
			template <typename... Args>
			BSFError(std::format_string<Args...> fmt, Args&&... args):
			Exception("BSF: ", fmt, std::forward<Args>(args)...) {}

			using FFmpeg::Exception::Exception;
	};

	/**
	 * @class DecoderError
	 * @brief The exception for when a decoder error occurs.
	 */
	class STORMBYTE_MULTIMEDIA_ADVANCED DecoderError: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The message of the exception.
			 */
			template <typename... Args>
			DecoderError(std::format_string<Args...> fmt, Args&&... args):
			Exception("Decoder: ", fmt, std::forward<Args>(args)...) {}

			using FFmpeg::Exception::Exception;
	};

	/**
	 * @class EncoderError
	 * @brief The exception for when an encoder error occurs.
	 */
	class STORMBYTE_MULTIMEDIA_ADVANCED EncoderError: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The message of the exception.
			 */
			template <typename... Args>
			EncoderError(std::format_string<Args...> fmt, Args&&... args):
			Exception("Encoder: ", fmt, std::forward<Args>(args)...) {}

			using FFmpeg::Exception::Exception;
	};
}