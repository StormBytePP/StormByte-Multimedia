/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia is dual-licensed under the following terms:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You can redistribute it and/or modify it under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this software may be used under the terms of a
 *    commercial license agreement with the sole copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *    Contact the copyright holder for more information.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this StormByte-Multimedia. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/multimedia/visibility.h>

#include <string>
#include <utility>

/**
 * @namespace Property
 * @brief Video/audio property value types.
 */
namespace StormByte::Multimedia::Context::Property {
	/**
	 * @class Color
	 * @brief Pixel format and colorimetry strings.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Color final {
		public:
			/**
			 * Full color description.
			 * @param pix_fmt Pixel format name.
			 * @param range Color range name.
			 * @param space Color space name.
			 * @param primaries Color primaries name.
			 * @param transfer Transfer characteristics name.
			 */
			Color(const std::string& pix_fmt, const std::string& range, const std::string& space,
				const std::string& primaries, const std::string& transfer) noexcept;

			/**
			 * Minimal color description (primaries/transfer empty).
			 * @param pix_fmt Pixel format name.
			 * @param range Color range name.
			 * @param space Color space name.
			 */
			Color(std::string&& pix_fmt, std::string&& range, std::string&& space) noexcept;

			/**
			 * Copy constructor.
			 */
			Color(const Color& color) = default;

			/**
			 * Move constructor.
			 */
			Color(Color&& color) noexcept = default;

			/**
			 * Copy assignment.
			 */
			Color& operator=(const Color& color) = default;

			/**
			 * Move assignment.
			 */
			Color& operator=(Color&& color) noexcept = default;

			/**
			 * Destructor.
			 */
			virtual ~Color() noexcept = default;

			/**
			 * @return Pixel format name.
			 */
			const std::string& PixelFormat() const noexcept;

			/**
			 * @return Color range name.
			 */
			const std::string& Range() const noexcept;

			/**
			 * @return Color space name.
			 */
			const std::string& Space() const noexcept;

			/**
			 * @return Transfer characteristics name.
			 */
			const std::string& Transfer() const noexcept;

			/**
			 * @return Color primaries name.
			 */
			const std::string& Primaries() const noexcept;

			/**
			 * @return true if format/range/space look like HDR10-capable.
			 */
			bool IsHDR10Possible() const noexcept;

			/**
			 * @return true if format/range/space look like HLG-capable.
			 */
			bool IsHLGPossible() const noexcept;

		protected:
			std::string m_pix_fmt;		///< Pixel format
			std::string m_range;		///< Range
			std::string m_space;		///< Space
			std::string m_primaries;	///< Primaries
			std::string m_transfer;		///< Transfer
	};
}
