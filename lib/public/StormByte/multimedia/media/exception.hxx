#pragma once

#include <StormByte/multimedia/exception.hxx>

/**
 * @namespace StormByte::Multimedia::Media
 * @brief Public media types: codecs, registry and stream kinds.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @class Exception
	 * @brief Base exception for Media.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Exception: public Multimedia::Exception {
		public:
			/**
			 * @brief Constructs a formatted Media exception.
			 * @tparam Args Format argument types.
			 * @param component Subsystem name.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			Exception(const std::string& component, std::format_string<Args...> fmt, Args&&... args):
			Multimedia::Exception("Media::" + component, fmt, std::forward<Args>(args)...) {}

			using Multimedia::Exception::Exception;

			/**
			 * @brief Destructor.
			 */
			virtual ~Exception() noexcept = default;
	};

	/**
	 * @class CodecNotFoundException
	 * @brief Thrown when FindCodec does not resolve a name or FFmpeg id.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC CodecNotFoundException: public Exception {
		public:
			/**
			 * @brief Constructs the exception for @p codec.
			 * @tparam Args Unused; kept for symmetry with Exception.
			 * @param codec Name or FFmpeg id that was not found.
			 */
			template <typename... Args>
			CodecNotFoundException(const std::string& codec):
			Exception("Codec", "Codec {} not found", codec) {}
	};
}
