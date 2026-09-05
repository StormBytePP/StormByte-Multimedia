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

#include <StormByte/multimedia/media/typedefs.hxx>
#include <StormByte/multimedia/media/type.hxx>

#include <string_view>

/**
 * @namespace StormByte::Multimedia::Media
 * @brief Public media types: codecs, containers, registry and stream kinds.
 */
namespace StormByte::Multimedia::Media {
	class Registry;

	/**
	 * @class Container
	 * @brief Immutable container identity owned by StormByte::Multimedia::Media::Registry.
	 *
	 * Name() is the StormByte key. FFmpeg format ids live only in the registry map.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Container {
		public:
			/**
			 * @brief Copy is disabled; instances are unique in the registry.
			 */
			Container(const Container&) noexcept = delete;

			/**
			 * @brief Move constructor.
			 */
			Container(Container&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Container() noexcept = default;

			/**
			 * @brief Copy assignment is disabled.
			 * @return *this.
			 */
			Container& operator=(const Container&) noexcept = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Container& operator=(Container&&) noexcept = default;

			/**
			 * @brief Identity equality (same registry slot).
			 * @param other Other container.
			 * @return true if both refer to the same instance.
			 */
			bool operator==(const Container& other) const noexcept;

			/**
			 * @brief Identity inequality.
			 * @param other Other container.
			 * @return true if they are different instances.
			 */
			bool operator!=(const Container& other) const noexcept;

			/**
			 * @brief StormByte container name.
			 * @return View to a process-lifetime literal.
			 */
			constexpr std::string_view Name() const noexcept { return m_name; }

			/**
			 * @brief Human description.
			 * @return View to a process-lifetime literal.
			 */
			constexpr std::string_view Description() const noexcept { return m_description; }

			/**
			 * @brief Default file extension without a leading dot.
			 * @return View to a table literal, or empty if the row has none.
			 */
			constexpr std::string_view Extension() const noexcept { return m_extension; }

			/**
			 * @brief Tests Read/Write flags.
			 * @param access Flags to test.
			 * @return true if every bit in @p access is set.
			 */
			bool HasAccess(Access access) const noexcept;

			/**
			 * @brief Whether every codec in @p codecs is allowed in this container.
			 * @param codecs Codecs to test.
			 * @return true if @p codecs is empty or all entries are in the compatibility set.
			 */
			bool Allows(const CodecRefs& codecs) const noexcept;

		private:
			friend class Registry;

			std::string_view m_name;		///< StormByte name
			std::string_view m_description;		///< Description
			std::string_view m_extension;		///< Primary extension
			Access m_access;			///< Read and optional Write
			CodecRefs m_allowed;			///< Resolved compatibility set

			/**
			 * @brief Registry-only constructor.
			 * @param name StormByte name (table literal).
			 * @param description Description (table literal).
			 * @param extension Primary extension (table literal).
			 * @param access Capability mask.
			 */
			constexpr Container(std::string_view name, std::string_view description, std::string_view extension, Access access) noexcept
			: m_name(name), m_description(description), m_extension(extension), m_access(access) {}
	};
}
