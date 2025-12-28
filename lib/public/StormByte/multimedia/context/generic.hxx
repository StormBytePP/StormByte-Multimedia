#pragma once

#include <StormByte/clonable.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace Context
 * @brief The namespace for all context classes.
 */
namespace StormByte::Multimedia::Context {
	/**
	 * @class Generic
	 * @brief The Generic class for media contexts.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Generic: public StormByte::Clonable<Generic> {
		public:
			/**
			 * @brief Default constructor.
			 */
			Generic() noexcept											= default;

			/**
			 * @brief Copy constructor.
			 * @param other The other context to copy from.
			 */
			Generic(const Generic& other)								= default;

			/**
			 * @brief Move constructor.
			 * @param other The other context to move from.
			 */
			Generic(Generic&& other) noexcept							= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Generic() noexcept 								= default;

			/**
			 * @brief Copy assignment operator.
			 * @param other The other context to copy from.
			 * @return Reference to this context.
			 */
			Generic& operator=(const Generic& other)					= default;

			/**
			 * @brief Move assignment operator.
			 * @param other The other context to move from.
			 * @return Reference to this context.
			 */
			Generic& operator=(Generic&& other)							= default;
	};

}