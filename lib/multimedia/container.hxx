#pragma once

#include <multimedia/media/container.hxx>
#include <multimedia/stream.hxx>

#include <vector>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	class STORMBYTE_MULTIMEDIA_PUBLIC Container {
		public:
			/**
			 * @brief Default constructor.
			 */
			Container() noexcept;

			/**
			 * @brief Copy constructor.
			 * @param container The Container to copy.
			 */
			Container(const Container& container) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param container The Container to move.
			 */
			Container(Container&& container) noexcept = default;

			/**
			 * @brief Copy assignment operator.
			 * @param container The Container to copy.
			 * @return The copied Container.
			 */
			Container& operator=(const Container& container) noexcept = default;

			/**
			 * @brief Move assignment operator.
			 * @param container The Container to move.
			 * @return The moved Container.
			 */
			Container& operator=(Container&& container) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Container() noexcept = default;

		private:
			std::vector<Stream::PointerType> streams;
	};
}