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

#include <StormByte/bitmask.hxx>
#include <StormByte/multimedia/feature.hxx>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Features
	 * @brief Bitmask of Feature flags with ergonomic helpers.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Features: public StormByte::Bitmask<Features, Feature> {
		public:
			/**
			 * Default constructor (empty set).
			 */
			constexpr Features() noexcept: StormByte::Bitmask<Features, Feature>() {}

			/**
			 * @param feature Initial feature.
			 */
			constexpr Features(Feature feature) noexcept
			: StormByte::Bitmask<Features, Feature>(feature) {}

			/**
			 * Copy constructor.
			 */
			constexpr Features(const Features& other) noexcept = default;

			/**
			 * Move constructor.
			 */
			constexpr Features(Features&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			constexpr ~Features() noexcept = default;

			/**
			 * Copy assignment.
			 */
			constexpr Features& operator=(const Features& other) noexcept = default;

			/**
			 * Move assignment.
			 */
			constexpr Features& operator=(Features&& other) noexcept = default;

			/**
			 * Equality.
			 * @param other Other set.
			 * @return true if equal.
			 */
			[[nodiscard]]
			constexpr bool operator==(const Features& other) const noexcept {
				return Bitmask<Features, Feature>::operator==(other);
			}

			/**
			 * Inequality.
			 * @param other Other set.
			 * @return true if not equal.
			 */
			[[nodiscard]]
			constexpr bool operator!=(const Features& other) const noexcept {
				return Bitmask<Features, Feature>::operator!=(other);
			}

			/**
			 * Human-readable list of enabled features ("A | B | C").
			 */
			operator std::string() const noexcept;
	};
}
