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
	/**
	 * @class Metadata
	 * @brief Key/value metadata map (file or stream tags).
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Metadata final: public StormByte::Iterable<std::map<std::string, std::string>> {
		public:
			/**
			 * Default constructor.
			 */
			Metadata() noexcept = default;

			/**
			 * Copy constructor.
			 */
			Metadata(const Metadata& other) = default;

			/**
			 * Move constructor.
			 */
			Metadata(Metadata&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Metadata() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Metadata& operator=(const Metadata& other) = default;

			/**
			 * Move assignment.
			 */
			Metadata& operator=(Metadata&& other) = default;
	};
}
