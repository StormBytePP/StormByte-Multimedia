#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class OGA
	 * @brief The OGA container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC OGA final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			OGA();

			/**
			 * @brief Copy constructor.
			 * @param oga The OGA to copy.
			 */
			OGA(const OGA& oga) 										= default;

			/**
			 * @brief Move constructor.
			 * @param oga The OGA to move.
			 */
			OGA(OGA&& oga) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param oga The OGA to copy.
			 * @return The copied OGA.
			 */
			OGA& operator=(const OGA& oga) 								= default;

			/**
			 * @brief Move assignment operator.
			 * @param oga The OGA to move.
			 * @return The moved OGA.
			 */
			OGA& operator=(OGA&& oga) noexcept 							= default;

			/**
			 * @brief Default destructor.
			 */
			~OGA() noexcept override									= default;

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