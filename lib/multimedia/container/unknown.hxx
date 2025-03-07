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
			Unknown(const Unknown& unknown) 										= default;

			/**
			 * @brief Move constructor.
			 * @param unknown The Unknown to move.
			 */
			Unknown(Unknown&& unknown) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param unknown The Unknown to copy.
			 * @return The copied Unknown.
			 */
			Unknown& operator=(const Unknown& unknown) 								= default;

			/**
			 * @brief Move assignment operator.
			 * @param unknown The Unknown to move.
			 * @return The moved Unknown.
			 */
			Unknown& operator=(Unknown&& unknown) noexcept 							= default;

			/**
			 * @brief Default destructor.
			 */
			~Unknown() noexcept override											= default;

			/**
			 * @brief Gets the compatible streams with the container.
			 * @return The compatible streams with the container.
			 */
			std::list<StormByte::Multimedia::Property::Type> 						CompatibleStreams() const noexcept override;

			/**
			 * @brief Checks if the codec is compatible with the container.
			 * @param codec The codec to check.
			 * @return True if the codec is compatible with the container, false otherwise.
			 */
			bool 														IsCodecCompatible(const Codec::Base& codec) const noexcept override;
	};
}