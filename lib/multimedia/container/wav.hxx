#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class WAV
	 * @brief The WAV container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC WAV final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			WAV();

			/**
			 * @brief Copy constructor.
			 * @param wav The WAV to copy.
			 */
			WAV(const WAV& wav) 										= default;

			/**
			 * @brief Move constructor.
			 * @param wav The WAV to move.
			 */
			WAV(WAV&& wav) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param wav The WAV to copy.
			 * @return The copied WAV.
			 */
			WAV& operator=(const WAV& wav) 								= default;

			/**
			 * @brief Move assignment operator.
			 * @param wav The WAV to move.
			 * @return The moved WAV.
			 */
			WAV& operator=(WAV&& wav) noexcept 							= default;

			/**
			 * @brief Default destructor.
			 */
			~WAV() noexcept override									= default;

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