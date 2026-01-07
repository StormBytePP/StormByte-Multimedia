#pragma once

#include <StormByte/iterable.hxx>
#include <StormByte/multimedia/engine/stream.hxx>

#include <memory>
#include <vector>

/**
 * @namespace Engine
 * @brief The namespace for all multimedia engine classes.
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Streams
	 * @brief The class representing a collection of multimedia streams.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Streams final: public Iterable<std::vector<std::shared_ptr<Stream>>> {
		public:
			/**
			 * @brief Constructor.
			 */
			Streams() noexcept									= default;

			/**
			 * @brief Copy constructor.
			 * @param other The other streams to copy from.
			 */
			Streams(const Streams& other)						= default;

			/**
			 * @brief Move constructor.
			 * @param other The other streams to move from.
			 */
			Streams(Streams&& other) noexcept					= default;

			/**
			 * @brief Default destructor.
			 */
			~Streams() noexcept									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other streams to copy from.
			 * @return Reference to this streams.
			 */
			Streams& operator=(const Streams& other)			= default;

			/**
			 * @brief Move assignment operator.
			 * @param other The other streams to move from.
			 * @return Reference to this streams.
			 */
			Streams& operator=(Streams&& other) noexcept		= default;
	};
}