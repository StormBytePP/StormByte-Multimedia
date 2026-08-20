#pragma once

#include <StormByte/multimedia/visibility.h>

#include <string>

/**
 * @namespace Property
 * @brief Video/audio property value types.
 */
namespace StormByte::Multimedia::Context::Property {
	/**
	 * @class Resolution
	 * @brief Frame width and height with display helpers.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Resolution final {
		public:
			/**
			 * @param width Width in pixels.
			 * @param height Height in pixels.
			 */
			Resolution(unsigned short width, unsigned short height);

			/**
			 * Copy constructor.
			 */
			Resolution(const Resolution& resolution) = default;

			/**
			 * Move constructor.
			 */
			Resolution(Resolution&& resolution) noexcept = default;

			/**
			 * Copy assignment.
			 */
			Resolution& operator=(const Resolution& resolution) = default;

			/**
			 * Move assignment.
			 */
			Resolution& operator=(Resolution&& resolution) noexcept = default;

			/**
			 * Destructor.
			 */
			~Resolution() noexcept = default;

			/**
			 * @return Width in pixels.
			 */
			unsigned short Width() const noexcept;

			/**
			 * @return Height in pixels.
			 */
			unsigned short Height() const noexcept;

			/**
			 * @return "WIDTHxHEIGHT" string.
			 */
			std::string Name() const noexcept;

			/**
			 * @return Coarse label (e.g. "1080p", "4K").
			 */
			std::string StandardName() const noexcept;

		private:
			unsigned short m_width;		///< Width
			unsigned short m_height;	///< Height
	};
}
