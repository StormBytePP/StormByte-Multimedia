#pragma once

#include <multimedia/container/container.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class WebM
	 * @brief The WebM container class.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC WebM final: public Container {
		public:
			/**
			 * @brief Default constructor.
			 */
			WebM();

			/**
			 * @brief Copy constructor.
			 * @param webm The WebM to copy.
			 */
			WebM(const WebM& webm) 										= default;

			/**
			 * @brief Move constructor.
			 * @param webm The WebM to move.
			 */
			WebM(WebM&& webm) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param webm The WebM to copy.
			 * @return The copied WebM.
			 */
			WebM& operator=(const WebM& webm) 							= default;

			/**
			 * @brief Move assignment operator.
			 * @param webm The WebM to move.
			 * @return The moved WebM.
			 */
			WebM& operator=(WebM&& webm) noexcept 						= default;

			/**
			 * @brief Default destructor.
			 */
			~WebM() noexcept override									= default;

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}