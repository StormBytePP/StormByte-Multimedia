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