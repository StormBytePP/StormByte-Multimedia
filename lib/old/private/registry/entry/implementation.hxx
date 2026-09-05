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
