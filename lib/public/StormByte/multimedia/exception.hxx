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
	 * @brief Base exception for the multimedia module.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Exception: public StormByte::Exception {
		public:
			/**
			 * @tparam Args Format argument types.
			 * @param component Subsystem name.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			Exception(const std::string& component, std::format_string<Args...> fmt, Args&&... args):
			StormByte::Exception("Multimedia::" + component, fmt, std::forward<Args>(args)...) {}

			using StormByte::Exception::Exception;

			/**
			 * Destructor.
			 */
			virtual ~Exception() noexcept = default;
	};

	/**
	 * @class CodecNotFound
	 * @brief Thrown when a codec cannot be resolved.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC CodecNotFound: public Exception {
		public:
			/**
			 * @param codec Codec name or identifier string.
			 */
			CodecNotFound(const std::string& codec):
			Exception("Codec: ", "Codec '{}' not found.", codec) {}

			/**
			 * @param type Requested media type.
			 */
			CodecNotFound(Type type):
			Exception("Codec: ", "Codec of type '{}' not found.", ToString(type)) {}

			/**
			 * @param type Requested media type.
			 * @param features Required feature set.
			 */
			CodecNotFound(Type type, const Features& features):
			Exception("Codec: ", "Codec of type '{}' with features '{}' not found.", ToString(type), std::string(features)) {}
	};
}
