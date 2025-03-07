#pragma once

#include <multimedia/container/container.hxx>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	/**
	 * @class Matroska
	 * @brief The Matroska container class.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Matroska final: public Container {
		public:
			/**
			 * @brief Default constructor.
			 */
			Matroska();

			/**
			 * @brief Copy constructor.
			 * @param matroska The Matroska to copy.
			 */
			Matroska(const Matroska& matroska) 										= default;

			/**
			 * @brief Move constructor.
			 * @param matroska The Matroska to move.
			 */
			Matroska(Matroska&& matroska) noexcept 									= default;

			/**
			 * @brief Copy assignment operator.
			 * @param matroska The Matroska to copy.
			 * @return The copied Matroska.
			 */
			Matroska& operator=(const Matroska& matroska) 							= default;

			/**
			 * @brief Move assignment operator.
			 * @param matroska The Matroska to move.
			 * @return The moved Matroska.
			 */
			Matroska& operator=(Matroska&& matroska) noexcept 						= default;

			/**
			 * @brief Default destructor.
			 */
			~Matroska() noexcept override											= default;

		private:
			bool IsStreamCompatible(const Stream::Stream& stream) override;
	};
}