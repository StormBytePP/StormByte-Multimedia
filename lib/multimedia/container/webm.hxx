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
			std::list<StormByte::Multimedia::Property::Type> 			CompatibleStreams() const noexcept override;

			/**
			 * @brief Checks if the codec is compatible with the container.
			 * @param codec The codec to check.
			 * @return True if the codec is compatible with the container, false otherwise.
			 */
			bool 														IsCodecCompatible(const Codec::Base& codec) const noexcept override;
	};
}