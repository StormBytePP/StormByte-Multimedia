#pragma once

#include <StormByte/multimedia/engine/backend/typedefs.hxx>
#include <StormByte/multimedia/engine/streams.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>
#include <tuple>

/**
 * @namespace Backend
 * @brief Internal demuxer backends.
 */
namespace StormByte::Multimedia::Engine::Backend {
	/**
	 * @class Demuxer
	 * @brief Abstract backend demuxer (Open → metadata + streams).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Demuxer {
		public:
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
			virtual ~Demuxer() noexcept = default;

			/**
			 * Copy assignment (deleted).
			 */
			Demuxer& operator=(const Demuxer& other) = delete;

			/**
			 * Move assignment (deleted).
			 */
			Demuxer& operator=(Demuxer&& other) noexcept = delete;

			/**
			 * Opens @p file and extracts container metadata and streams.
			 * @param file Path to the media file.
			 * @return Metadata + streams, or DemuxerException.
			 */
			virtual ExpectedDemuxerTuple Open(const std::filesystem::path& file) const noexcept = 0;

		protected:
			/**
			 * Protected default constructor for derived backends.
			 */
			Demuxer() noexcept = default;
	};
}
