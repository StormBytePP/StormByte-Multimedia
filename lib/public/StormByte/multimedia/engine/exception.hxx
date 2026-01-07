#pragma once

#include <StormByte/multimedia/exception.hxx>

#include <filesystem>

/**
 * @namespace Engine
 * @brief The namespace for all multimedia engine classes.
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Exception
	 * @brief Base class for engine-related exceptions.
	 *
	 * This exception type is specific to the multimedia engine and extends
	 * `StormByte::Multimedia::Exception` by prepending the engine component
	 * name to the error category.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Exception: public StormByte::Multimedia::Exception {
		public:
			/**
			 * @brief Construct an engine exception with a formatted message.
			 * @tparam Args Types of the format arguments.
			 * @param component Name of the engine component (e.g. "Demuxer").
			 * @param fmt A `std::format`-style format string used to build the message.
			 * @param args Arguments forwarded to `std::format` to produce the message.
			 *
			 * If no `args` are provided the `fmt` string is used verbatim as the
			 * exception message.
			 */
			template <typename... Args>
			Exception(const std::string& component, std::format_string<Args...> fmt, Args&&... args):
			StormByte::Multimedia::Exception("Engine::" + component, fmt, std::forward<Args>(args)...) {}

			/**
			 * @brief Default destructor.
			 */
			virtual ~Exception() noexcept 								= default;
	};

	/**
	 * @class BackendNotImplemented
	 * @brief Exception indicating a requested backend is not implemented.
	 *
	 * Thrown when an attempt is made to use a multimedia engine backend
	 * that has not been implemented in the current build of the library.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC BackendNotImplemented: public Exception {
		public:
			/**
			 * @brief Construct a backend not implemented exception.
			 * @param backend Name of the backend that is not implemented.
			 */
			explicit BackendNotImplemented(const std::string& backend):
			Exception("Backend", "The backend '{}' is not implemented.", backend) {}
	};

	/**
	 * @class DemuxerException
	 * @brief Base exception for demuxer-related errors.
	 *
	 * This class is a convenience specialization of `Engine::Exception` for
	 * errors originating from the demuxer component.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC DemuxerException: public Exception {
		public:
			/**
			 * @brief Construct a demuxer exception with a formatted message.
			 * @tparam Args Types of the format arguments.
			 * @param fmt A `std::format`-style format string used to build the message.
			 * @param args Arguments forwarded to `std::format` to produce the message.
			 *
			 * The component name is set to "Demuxer" automatically.
			 */
			template <typename... Args>
			DemuxerException(std::format_string<Args...> fmt, Args&&... args):
			Exception("Demuxer", fmt, std::forward<Args>(args)...) {}

			/**
			 * @brief Default destructor.
			 */
			virtual ~DemuxerException() noexcept 								= default;
	};

	/**
	 * @class ReadError
	 * @brief Exception indicating a file cannot be read or does not exist.
	 *
	 * Thrown when the demuxer cannot open or read the specified file. The
	 * exception message includes the file path.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC ReadError: public DemuxerException {
		public:
			/**
			 * @brief Construct a read error for the given file.
			 * @param file Path to the file that could not be read.
			 */
			explicit ReadError(const std::filesystem::path& file):
			DemuxerException("File {} is not readable or it does not exist.", file.string()) {}
	};

	/**
	 * @class ContentError
	 * @brief Exception indicating the file is not a supported multimedia file.
	 *
	 * Thrown when the demuxer determines the file does not contain
	 * recognizable multimedia content.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC ContentError: public DemuxerException {
		public:
			/**
			 * @brief Construct a content error for the given file.
			 * @param file Path to the file with unsupported or unrecognized content.
			 */
			explicit ContentError(const std::filesystem::path& file):
			DemuxerException("File {} is not a multimedia file.", file.string()) {}
	};
}