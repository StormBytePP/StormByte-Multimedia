#pragma once

#include <multimedia/codec.hxx>
#include <multimedia/types.hxx>

#include <vector>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	template<StreamType T> class STORMBYTE_MULTIMEDIA_PUBLIC Stream {
		public:
			/**
			 * @brief Default constructor.
			 */
			Stream() = default;

			/**
			 * @brief Copy constructor.
			 * @param stream The Stream to copy.
			 */
			Stream(const Stream& stream) 						= default;

			/**
			 * @brief Move constructor.
			 * @param stream The Stream to move.
			 */
			Stream(Stream&& stream) noexcept 					= default;

			/**
			 * @brief Copy assignment operator.
			 * @param stream The Stream to copy.
			 * @return The copied Stream.
			 */
			Stream& operator=(const Stream& stream) 			= default;

			/**
			 * @brief Move assignment operator.
			 * @param stream The Stream to move.
			 * @return The moved Stream.
			 */
			Stream& operator=(Stream&& stream) noexcept 		= default;

			/**
			 * @brief Default destructor.
			 */
			virtual ~Stream() noexcept 							= default;

			/**
			 * @brief Gets the type of the stream.
			 * @return The type of the stream.
			 */
			StreamType 											GetType() const noexcept {
				return T;
			}
		
		protected:
	};
}