#pragma once

#include <StormByte/multimedia/engine/backend/typedefs.hxx>
#include <StormByte/multimedia/engine/streams.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>
#include <tuple>

/**
 * @namespace Backend
 * @brief The namespace for all internal Backend related classes and functions.
 */
namespace StormByte::Multimedia::Engine::Backend {
	/**
	 * @class Demuxer
	 * @brief The class representing a multimedia demuxer.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Demuxer {
		public:
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
			virtual ~Demuxer() noexcept									= default;

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
			virtual ExpectedDemuxerTuple 								Open(const std::filesystem::path& file) const noexcept = 0;

		protected:
			/**
			 * @brief Protected constructor used by derived classes.
			 */
			Demuxer() noexcept 											= default;
	};
}