#pragma once

#include <multimedia/container/base.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class Matroska
	 * @brief The Matroska container class.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Matroska final: public Base {
		public:
			/**
			 * @brief Default constructor.
			 */
			Matroska();

			/**
			 * @brief Copy constructor.
			 * @param matroska The Matroska to copy.
			 */
			Matroska(const Matroska& matroska) 							= default;

			/**
			 * @brief Move constructor.
			 * @param matroska The Matroska to move.
			 */
			Matroska(Matroska&& matroska) noexcept 						= default;

			/**
			 * @brief Copy assignment operator.
			 * @param matroska The Matroska to copy.
			 * @return The copied Matroska.
			 */
			Matroska& operator=(const Matroska& matroska) 				= default;

			/**
			 * @brief Move assignment operator.
			 * @param matroska The Matroska to move.
			 * @return The moved Matroska.
			 */
			Matroska& operator=(Matroska&& matroska) noexcept 			= default;

			/**
			 * @brief Default destructor.
			 */
			~Matroska() noexcept override								= default;

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