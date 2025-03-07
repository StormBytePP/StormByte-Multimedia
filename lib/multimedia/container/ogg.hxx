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