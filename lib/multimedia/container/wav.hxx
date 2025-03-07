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
			const CompatibleStreams& 									GetCompatibleStreams() const noexcept override;

			/**
			 * @brief Gets the compatible codecs with the container.
			 * @return The compatible codecs with the container.
			 */
			const CompatibleCodecs& 									GetCompatibleCodecs() const noexcept override;

		private:
			static const CompatibleStreams 	CompatStreams;				///< The compatible streams with the container.
			static const CompatibleCodecs 	CompatCodecs;				///< The compatible codecs with the container.
	};
}