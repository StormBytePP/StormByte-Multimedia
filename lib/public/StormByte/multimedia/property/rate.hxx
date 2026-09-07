/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia original source is dual-licensed:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You may redistribute and/or modify this file under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this file may be used under the terms of a commercial
 *    license agreement with the copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *
 * Both licenses apply only to original StormByte-Multimedia source in this
 * file. Third-party components — including FFmpeg and embedded trained data —
 * remain under their own licenses and are not covered by the commercial grant.
 *
 * Neither license grants any patent rights. Any patent licenses required
 * to use this software or third-party components must be obtained separately
 * from the patent holders.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with StormByte-Multimedia. If not, see
 * <https://www.gnu.org/licenses/lgpl-3.0.html>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-StormByte-Commercial
 */

#pragma once

#include <StormByte/multimedia/visibility.h>

/**
 * @namespace StormByte::Multimedia::Property
 * @brief Media property value types.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @class Rate
	 * @brief Rational rate. For video, Num()/Den() is frames per second.
	 *
	 * 23.976 fps is `{24000, 1001}`. 24 fps is `{24, 1}`.
	 * This is not an encoder time base and not a Matroska tick rate.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Rate final {
		public:
			/**
			 * @brief Constructs a rate.
			 * @param num Numerator. For fps, frame count.
			 * @param den Denominator. For fps, seconds. If @p den is &lt;= 0, stores `{0, 1}`.
			 */
			Rate(int num, int den) noexcept;

			/**
			 * @brief Copy constructor.
			 */
			Rate(const Rate&) = default;

			/**
			 * @brief Move constructor.
			 */
			Rate(Rate&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Rate() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @return *this.
			 */
			Rate& operator=(const Rate&) = default;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Rate& operator=(Rate&&) noexcept = default;

			/**
			 * @brief Numerator.
			 * @return Numerator.
			 */
			int Num() const noexcept;

			/**
			 * @brief Denominator.
			 * @return Denominator, always &gt; 0.
			 */
			int Den() const noexcept;

			/**
			 * @brief Whether the rate can be used as fps.
			 * @return true if both terms are positive.
			 */
			bool Valid() const noexcept;

		private:
			int m_num;	///< Numerator
			int m_den;	///< Denominator
	};
}
