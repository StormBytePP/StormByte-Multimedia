#pragma once

#include <multimedia/container/container.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class OGG
	 * @brief The OGG container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC OGG final: public Container {
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

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}