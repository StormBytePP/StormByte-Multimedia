#pragma once

#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <string>

/**
 * @namespace Media
 * @brief The namespace for all multimedia media types.
 */
namespace StormByte::Multimedia::Media {
	/**
	 * @struct Size
	 * @brief The struct for size properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Size {
		public:
			/**
			 * @brief Constructor.
			 * @param bytes The size in bytes.
			 */
			constexpr Size(uint64_t bytes): s_bytes(bytes) {}
			
			/**
			 * @brief Copy constructor.
			 * @param size The Size to copy.
			 */
			constexpr Size(const Size& size) 					= default;

			/**
			 * @brief Move constructor.
			 * @param size The Size to move.
			 */
			constexpr Size(Size&& size) noexcept 				= default;

			/**
			 * @brief Copy assignment operator.
			 * @param size The Size to copy.
			 * @return The copied Size.
			 */
			constexpr Size& operator=(const Size& size) 		= default;

			/**
			 * @brief Move assignment operator.
			 * @param size The Size to move.
			 * @return The moved Size.
			 */
			constexpr Size& operator=(Size&& size) noexcept 	= default;

			/**
			 * @brief Default destructor.
			 */
			constexpr ~Size() noexcept 							= default;

			/**
			 * @brief Gets the size in human readable format.
			 * @return The size in human readable format.
			 */
			std::string ToString() const noexcept;

		private:
			uint64_t s_bytes;									///< The size in bytes.
	};
}