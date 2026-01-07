#pragma once

#include <StormByte/exception.hxx>
#include <StormByte/multimedia/features.hxx>
#include <StormByte/multimedia/type.hxx>

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

			/**
			 * @brief Constructor with type
			 * @param type The type of the codec.
			 */
			CodecNotFound(Type type):
			Exception("Codec: ", "Codec of type '{}' not found.", ToString(type)) {}

			/**
			 * @brief Constructor with type and features.
			 * @param type The type of the codec.
			 * @param features The features of the codec.
			 */
			CodecNotFound(Type type, const Features& features):
			Exception("Codec: ", "Codec of type '{}' with features '{}' not found.", ToString(type), std::string(features)) {}
	};
}