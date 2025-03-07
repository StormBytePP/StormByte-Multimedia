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
			std::list<StormByte::Multimedia::Property::Type> 			CompatibleStreams() const noexcept override;

			/**
			 * @brief Checks if the codec is compatible with the container.
			 * @param codec The codec to check.
			 * @return True if the codec is compatible with the container, false otherwise.
			 */
			bool 														IsCodecCompatible(const Codec::Base& codec) const noexcept override;
	};
}