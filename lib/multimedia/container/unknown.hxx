#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class Unknown
	 * @brief The Unknown container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Unknown final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			Unknown();

			/**
			 * @brief Copy constructor.
			 * @param unknown The Unknown to copy.
			 */
			Unknown(const Unknown& unknown) 							= default;

			/**
			 * @brief Move constructor.
			 * @param unknown The Unknown to move.
			 */
			Unknown(Unknown&& unknown) noexcept 						= default;

			/**
			 * @brief Copy assignment operator.
			 * @param unknown The Unknown to copy.
			 * @return The copied Unknown.
			 */
			Unknown& operator=(const Unknown& unknown) 					= default;

			/**
			 * @brief Move assignment operator.
			 * @param unknown The Unknown to move.
			 * @return The moved Unknown.
			 */
			Unknown& operator=(Unknown&& unknown) noexcept 				= default;

			/**
			 * @brief Default destructor.
			 */
			~Unknown() noexcept override								= default;

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