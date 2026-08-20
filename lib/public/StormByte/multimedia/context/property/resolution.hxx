/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

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
