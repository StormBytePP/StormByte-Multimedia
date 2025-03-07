#pragma once

#include <multimedia/container/container.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class OGV
	 * @brief The OGV container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC OGV final: public Container {
		public:
			/**
			 * @brief Default constructor.
			 */
			OGV();

			/**
			 * @brief Copy constructor.
			 * @param ogv The OGV to copy.
			 */
			OGV(const OGV& ogv) 										= default;

			/**
			 * @brief Move constructor.
			 * @param ogv The OGV to move.
			 */
			OGV(OGV&& ogv) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param ogv The OGV to copy.
			 * @return The copied OGV.
			 */
			OGV& operator=(const OGV& ogv) 								= default;

			/**
			 * @brief Move assignment operator.
			 * @param ogv The OGV to move.
			 * @return The moved OGV.
			 */
			OGV& operator=(OGV&& ogv) noexcept 							= default;

			/**
			 * @brief Default destructor.
			 */
			~OGV() noexcept override									= default;

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}