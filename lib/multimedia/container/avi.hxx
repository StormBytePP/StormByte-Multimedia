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