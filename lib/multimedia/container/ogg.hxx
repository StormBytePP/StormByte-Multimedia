#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class OGG
	 * @brief The OGG container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC OGG final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			OGG();

			/**
			 * @brief Copy constructor.
			 * @param ogg The OGG to copy.
			 */
			OGG(const OGG& ogg) 										= default;

			/**
			 * @brief Move constructor.
			 * @param ogg The OGG to move.
			 */
			OGG(OGG&& ogg) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param ogg The OGG to copy.
			 * @return The copied OGG.
			 */
			OGG& operator=(const OGG& ogg) 								= default;

			/**
			 * @brief Move assignment operator.
			 * @param ogg The OGG to move.
			 * @return The moved OGG.
			 */
			OGG& operator=(OGG&& ogg) noexcept 							= default;

			/**
			 * @brief Default destructor.
			 */
			~OGG() noexcept override									= default;

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