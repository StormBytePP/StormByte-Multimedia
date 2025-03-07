#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class FLAC
	 * @brief The FLAC container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC FLAC final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			FLAC();

			/**
			 * @brief Copy constructor.
			 * @param flac The FLAC to copy.
			 */
			FLAC(const FLAC& flac) 										= default;

			/**
			 * @brief Move constructor.
			 * @param flac The FLAC to move.
			 */
			FLAC(FLAC&& flac) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param flac The FLAC to copy.
			 * @return The copied FLAC.
			 */
			FLAC& operator=(const FLAC& flac) 							= default;

			/**
			 * @brief Move assignment operator.
			 * @param flac The FLAC to move.
			 * @return The moved FLAC.
			 */
			FLAC& operator=(FLAC&& flac) noexcept 						= default;

			/**
			 * @brief Default destructor.
			 */
			~FLAC() noexcept override									= default;

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