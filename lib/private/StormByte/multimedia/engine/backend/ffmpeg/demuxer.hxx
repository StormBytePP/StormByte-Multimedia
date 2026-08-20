#pragma once

#include <StormByte/multimedia/engine/backend/demuxer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFormatContext.hxx>

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class Demuxer
	 * @brief FFmpeg implementation of Backend::Demuxer.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Demuxer final: public Backend::Demuxer {
		public:
			/**
			 * Default constructor.
			 */
			Demuxer() noexcept = default;

			/**
			 * Copy constructor (deleted).
			 */
			Demuxer(const Demuxer& other) = delete;

			/**
			 * Move constructor (deleted).
			 */
			Demuxer(Demuxer&& other) noexcept = delete;

			/**
			 * Destructor.
			 */
			~Demuxer() noexcept override = default;

			/**
			 * Copy assignment (deleted).
			 */
			Demuxer& operator=(const Demuxer& other) = delete;

			/**
			 * Move assignment (deleted).
			 */
			Demuxer& operator=(Demuxer&& other) noexcept = delete;

			/**
			 * Opens @p file via AVFormatContext and builds Engine streams (incl. HDR probe).
			 * @param file Media path.
			 * @return Metadata + streams, or error.
			 */
			ExpectedDemuxerTuple Open(const std::filesystem::path& file) const noexcept override;
	};
}
