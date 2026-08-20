#pragma once

#include <StormByte/iterable.hxx>
#include <StormByte/multimedia/engine/stream.hxx>

#include <memory>
#include <vector>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Streams
	 * @brief Ordered collection of Stream shared_ptrs.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Streams final: public Iterable<std::vector<std::shared_ptr<Stream>>> {
		public:
			/**
			 * Default constructor.
			 */
			Streams() noexcept = default;

			/**
			 * Copy constructor.
			 */
			Streams(const Streams& other) = default;

			/**
			 * Move constructor.
			 */
			Streams(Streams&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Streams() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Streams& operator=(const Streams& other) = default;

			/**
			 * Move assignment.
			 */
			Streams& operator=(Streams&& other) noexcept = default;
	};
}
