#pragma once

#include <StormByte/multimedia/engine/implementation.hxx>
#include <StormByte/multimedia/engine/typedefs.hxx>
#include <StormByte/multimedia/engine/streams.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Demuxer
	 * @brief Opened multimedia file: path, container metadata and streams.
	 *
	 * Non-copyable; move-only. Create via Open().
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Demuxer final {
		public:
			/**
			 * Copy constructor (deleted).
			 */
			Demuxer(const Demuxer& other) = delete;

			/**
			 * Move constructor.
			 */
			Demuxer(Demuxer&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Demuxer() noexcept = default;

			/**
			 * Copy assignment (deleted).
			 */
			Demuxer& operator=(const Demuxer& other) = delete;

			/**
			 * Move assignment.
			 */
			Demuxer& operator=(Demuxer&& other) noexcept = default;

			/**
			 * @return Path of the opened file.
			 */
			const std::filesystem::path& File() const noexcept;

			/**
			 * @return Container-level metadata.
			 */
			inline const class Metadata& Metadata() const noexcept {
				return m_metadata;
			}

			/**
			 * @return Discovered streams.
			 */
			inline const class Streams& Streams() const noexcept {
				return m_streams;
			}

			/**
			 * Opens a file with the given backend.
			 * @param path File path.
			 * @param implementation Backend to use.
			 * @return Demuxer or engine exception.
			 */
			static ExpectedDemuxer Open(const std::filesystem::path& path, enum Implementation implementation) noexcept;

		private:
			std::filesystem::path m_path;		///< File path
			class Metadata m_metadata;			///< Container metadata
			class Streams m_streams;			///< Streams

			/**
			 * @param path File path.
			 * @param metadata Container metadata.
			 * @param streams Stream list.
			 */
			Demuxer(const std::filesystem::path& path, class Metadata&& metadata, class Streams&& streams) noexcept;
	};
}
