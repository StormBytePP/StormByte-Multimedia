#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class Opus
	 * @brief The Opus container.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Opus final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			Opus();

			/**
			 * @brief Copy constructor.
			 * @param opus The Opus to copy.
			 */
			Opus(const Opus& opus) 										= default;

			/**
			 * @brief Move constructor.
			 * @param opus The Opus to move.
			 */
			Opus(Opus&& opus) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param opus The Opus to copy.
			 * @return The copied Opus.
			 */
			Opus& operator=(const Opus& opus) 							= default;

			/**
			 * @brief Move assignment operator.
			 * @param opus The Opus to move.
			 * @return The moved Opus.
			 */
			Opus& operator=(Opus&& opus) noexcept 						= default;

			/**
			 * @brief Default destructor.
			 */
			~Opus() noexcept override									= default;

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