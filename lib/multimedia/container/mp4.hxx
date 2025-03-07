#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class MP4
	 * @brief The MP4 container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC MP4 final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			MP4();

			/**
			 * @brief Copy constructor.
			 * @param mp4 The MP4 to copy.
			 */
			MP4(const MP4& mp4) 										= default;

			/**
			 * @brief Move constructor.
			 * @param mp4 The MP4 to move.
			 */
			MP4(MP4&& mp4) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param mp4 The MP4 to copy.
			 * @return The copied MP4.
			 */
			MP4& operator=(const MP4& mp4) 								= default;

			/**
			 * @brief Move assignment operator.
			 * @param mp4 The MP4 to move.
			 * @return The moved MP4.
			 */
			MP4& operator=(MP4&& mp4) noexcept 							= default;

			/**
			 * @brief Default destructor.
			 */
			~MP4() noexcept override									= default;

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