#pragma once

#include <multimedia/container/container.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class OGA
	 * @brief The OGA container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC OGA final: public Container {
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

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}