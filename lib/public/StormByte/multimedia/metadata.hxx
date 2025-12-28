#pragma once

#include <StormByte/iterable.hxx>
#include <StormByte/multimedia/visibility.h>

#include <map>
#include <string>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	class STORMBYTE_MULTIMEDIA_PUBLIC Metadata final: public StormByte::Iterable<std::map<std::string, std::string>> {
		public:
			/**
			 * @brief Default constructor.
			 */
			Metadata() noexcept										= default;

			/**
			 * @brief Copy constructor.
			 * @param other The other metadata to copy from.
			 */
			Metadata(const Metadata& other)							= default;

			/**
			 * @brief Move constructor.
			 * @param other The other metadata to move from.
			 */
			Metadata(Metadata&& other) noexcept						= default;

			/**
			 * @brief Default destructor.
			 */
			~Metadata() noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other metadata to copy from.
			 * @return Reference to this metadata.
			 */
			Metadata& operator=(const Metadata& other)				= default;

			/**
			 * @brief Move assignment operator.
			 * @param other The other metadata to move from.
			 * @return Reference to this metadata.
			 */
			Metadata& operator=(Metadata&& other)					= default;
	};
}