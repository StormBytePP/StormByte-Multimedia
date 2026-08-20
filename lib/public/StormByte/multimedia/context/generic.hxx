#pragma once

#include <StormByte/clonable.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Context
 * @brief Media stream context types (audio, video, …).
 */
namespace StormByte::Multimedia::Context {
	/**
	 * @class Generic
	 * @brief Base class for stream contexts (Clonable).
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Generic: public StormByte::Clonable<Generic> {
		public:
			/**
			 * Default constructor.
			 */
			Generic() noexcept = default;

			/**
			 * Copy constructor.
			 */
			Generic(const Generic& other) = default;

			/**
			 * Move constructor.
			 */
			Generic(Generic&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			virtual ~Generic() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Generic& operator=(const Generic& other) = default;

			/**
			 * Move assignment.
			 */
			Generic& operator=(Generic&& other) = default;
	};
}
