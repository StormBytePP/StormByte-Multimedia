#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class WebM
	 * @brief The WebM container class.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC WebM final: public Base {
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