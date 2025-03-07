#pragma once

#include <multimedia/container/container.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class Unknown
	 * @brief The Unknown container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Unknown final: public Container {
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

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}