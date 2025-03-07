#pragma once

#include <multimedia/container/container.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class MP3
	 * @brief The MP3 container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC MP3 final: public Container {
		public:
			/**
			 * @brief Default constructor.
			 */
			MP3();

			/**
			 * @brief Copy constructor.
			 * @param mp3 The MP3 to copy.
			 */
			MP3(const MP3& mp3) 										= default;

			/**
			 * @brief Move constructor.
			 * @param mp3 The MP3 to move.
			 */
			MP3(MP3&& mp3) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param mp3 The MP3 to copy.
			 * @return The copied MP3.
			 */
			MP3& operator=(const MP3& mp3) 								= default;

			/**
			 * @brief Move assignment operator.
			 * @param mp3 The MP3 to move.
			 * @return The moved MP3.
			 */
			MP3& operator=(MP3&& mp3) noexcept 							= default;

			/**
			 * @brief Default destructor.
			 */
			~MP3() noexcept override									= default;

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}