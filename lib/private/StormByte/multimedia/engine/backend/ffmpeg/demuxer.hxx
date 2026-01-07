#pragma once

#include <StormByte/multimedia/engine/backend/demuxer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFormatContext.hxx>

/**
 * @namespace FFmpeg
 * @brief The namespace for all internal FFmpeg related classes and functions.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class Demuxer
	 * @brief The class representing a multimedia demuxer.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Demuxer final: public Backend::Demuxer {
		public:
			/**
			 * @brief Constructor.
			 */
			Demuxer() noexcept											= default;

			/**
			 * @brief Copy constructor.
			 * @param other The other file to copy from.
			 */
			Demuxer(const Demuxer& other)								= delete;

			/**
			 * @brief Move constructor.
			 * @param other The other file to move from.
			 */
			Demuxer(Demuxer&& other) noexcept							= delete;

			/**
			 * @brief Default destructor.
			 */
			~Demuxer() noexcept override								= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other file to copy from.
			 * @return Reference to this file.
			 */
			Demuxer& operator=(const Demuxer& other)					= delete;

			/**
			 * @brief Move assignment operator.
			 * @param other The other file to move from.
			 * @return Reference to this file.
			 */
			Demuxer& operator=(Demuxer&& other) noexcept				= delete;

			/**
			 * @brief Opens demuxer for the specified file and gets its components.
			 * @param file The path to the multimedia file.
			 * @return Tuple containing path, metadata and streams or exception on failure.
			 */
			ExpectedDemuxerTuple 										Open(const std::filesystem::path& file) const noexcept override;
	};
}