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

#include <StormByte/multimedia/features.hxx>

/**
 * @namespace Implementation
 * @brief Registry entry for named decoder/encoder implementations.
 */
namespace StormByte::Multimedia::Registry::Entry::Implementation {
	/**
	 * @class Entry
	 * @brief Compile-time implementation name + feature set.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Entry {
		public:
			/**
			 * @param name Implementation name (e.g. "libx264").
			 * @param feature Single feature.
			 */
			constexpr Entry(const char* name, Feature feature) noexcept
			: m_name(name), m_features(feature) {}

			/**
			 * @param name Implementation name.
			 * @param features Feature bitmask.
			 */
			constexpr Entry(const char* name, Features features) noexcept
			: m_name(name), m_features(features) {}

			/**
			 * Copy constructor.
			 */
			constexpr Entry(const Entry& other) noexcept = default;

			/**
			 * Move constructor.
			 */
			constexpr Entry(Entry&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			constexpr ~Entry() noexcept = default;

			/**
			 * Copy assignment.
			 */
			constexpr Entry& operator=(const Entry& other) noexcept = default;

			/**
			 * Move assignment.
			 */
			constexpr Entry& operator=(Entry&& other) noexcept = default;

			/**
			 * @return Implementation name.
			 */
			[[nodiscard]] constexpr const char* Name() const noexcept {
				return m_name;
			}

			/**
			 * @return Feature set.
			 */
			[[nodiscard]] constexpr const class Features& Features() const noexcept {
				return m_features;
			}

		private:
			const char* m_name;				///< Name
			class Features m_features;		///< Capabilities
	};
}
