#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class MP3
	 * @brief The MP3 container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC MP3 final: public Base {
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

			/**
			 * @brief Gets the compatible streams with the container.
			 * @return The compatible streams with the container.
			 */
			inline const CompatibleStreams& 							GetCompatibleStreams() const noexcept override {
				return CompatStreams;
			}

			/**
			 * @brief Gets the compatible codecs with the container.
			 * @return The compatible codecs with the container.
			 */
			inline const CompatibleCodecs& 								GetCompatibleCodecs() const noexcept override {
				return CompatCodecs;
			}

		private:
			static const CompatibleStreams 	CompatStreams;				///< The compatible streams with the container.
			static const CompatibleCodecs 	CompatCodecs;				///< The compatible codecs with the container.
	};
}