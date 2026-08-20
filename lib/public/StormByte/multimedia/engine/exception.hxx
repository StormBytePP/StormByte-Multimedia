#pragma once

#include <StormByte/multimedia/exception.hxx>

#include <filesystem>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Exception
	 * @brief Base exception for engine components.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Exception: public StormByte::Multimedia::Exception {
		public:
			/**
			 * @tparam Args Format argument types.
			 * @param component Engine subsystem name.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			Exception(const std::string& component, std::format_string<Args...> fmt, Args&&... args):
			StormByte::Multimedia::Exception("Engine::" + component, fmt, std::forward<Args>(args)...) {}

			/**
			 * Destructor.
			 */
			virtual ~Exception() noexcept = default;
	};

	/**
	 * @class BackendNotImplemented
	 * @brief Requested backend is not available in this build.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC BackendNotImplemented: public Exception {
		public:
			/**
			 * @param backend Backend name.
			 */
			explicit BackendNotImplemented(const std::string& backend):
			Exception("Backend", "The backend '{}' is not implemented.", backend) {}
	};

	/**
	 * @class DemuxerException
	 * @brief Base for demuxer failures.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC DemuxerException: public Exception {
		public:
			/**
			 * @tparam Args Format argument types.
			 * @param fmt Format string.
			 * @param args Format arguments.
			 */
			template <typename... Args>
			DemuxerException(std::format_string<Args...> fmt, Args&&... args):
			Exception("Demuxer", fmt, std::forward<Args>(args)...) {}

			/**
			 * Destructor.
			 */
			virtual ~DemuxerException() noexcept = default;
	};

	/**
	 * @class ReadError
	 * @brief File missing or not readable.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC ReadError: public DemuxerException {
		public:
			/**
			 * @param file Path that failed.
			 */
			explicit ReadError(const std::filesystem::path& file):
			DemuxerException("File {} is not readable or it does not exist.", file.string()) {}
	};

	/**
	 * @class ContentError
	 * @brief File is not recognized as multimedia.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC ContentError: public DemuxerException {
		public:
			/**
			 * @param file Path that failed.
			 */
			explicit ContentError(const std::filesystem::path& file):
			DemuxerException("File {} is not a multimedia file.", file.string()) {}
	};
}
