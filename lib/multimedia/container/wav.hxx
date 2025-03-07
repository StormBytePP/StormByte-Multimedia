#pragma once

#include <multimedia/container/container.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class WAV
	 * @brief The WAV container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC WAV final: public Container {
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

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}