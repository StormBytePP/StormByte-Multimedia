#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class AVI
	 * @brief The AVI container class.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC AVI final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			AVI();

			/**
			 * @brief Copy constructor.
			 * @param avi The AVI to copy.
			 */
			AVI(const AVI& avi) 										= default;

			/**
			 * @brief Move constructor.
			 * @param avi The AVI to move.
			 */
			AVI(AVI&& avi) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param avi The AVI to copy.
			 * @return The copied AVI.
			 */
			AVI& operator=(const AVI& avi) 								= default;

			/**
			 * @brief Move assignment operator.
			 * @param avi The AVI to move.
			 * @return The moved AVI.
			 */
			AVI& operator=(AVI&& avi) noexcept 							= default;

			/**
			 * @brief Default destructor.
			 */
			~AVI() noexcept override									= default;

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}