#pragma once

#include <StormByte/exception.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Exception
	 * @brief The base exception for multimedia.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Exception: public StormByte::Exception {
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
			StormByte::Exception("Multimedia::" +component, fmt, std::forward<Args>(args)...) {}

			using StormByte::Exception::Exception;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Exception() noexcept 								= default;
	};

	/**
	 * @class CodecNotFound
	 * @brief The exception for when a codec is not found.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC CodecNotFound: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The message of the exception.
			 */
			CodecNotFound(const std::string& codec):
			Exception("Codec: ", "Codec '{}' not found.", codec) {}

			using Exception::Exception;
	};

	/**
	 * @class FileError
	 * @brief The exception for when a file error occurs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC FileError: public Exception {
		public:
			/**
			 * @brief Default constructor.
			 * @param codec The message of the exception.
			 */
			FileError(const std::string& message):
			Exception("File: ", "File error occurred: {}", message) {}

			using Exception::Exception;
	};
}