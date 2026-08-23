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

extern "C" {
	#include <libavcodec/avcodec.h>
}

/**
 * @namespace Codec
 * @brief Registry entry for logical codecs.
 */
namespace StormByte::Multimedia::Registry::Entry::Codec {
	/**
	 * @class Entry
	 * @brief Compile-time codec id + type + feature set.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Entry {
		public:
			/**
			 * @param id AVCodecID.
			 * @param type Media type.
			 * @param feature Single feature flag.
			 */
			constexpr Entry(AVCodecID id, enum Type type, Feature feature) noexcept
			: m_id(id), m_type(type), m_features(feature) {}

			/**
			 * @param id AVCodecID.
			 * @param type Media type.
			 * @param features Feature bitmask.
			 */
			constexpr Entry(AVCodecID id, enum Type type, Features features) noexcept
			: m_id(id), m_type(type), m_features(features) {}

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
			 * @return Codec id.
			 */
			constexpr AVCodecID ID() const noexcept {
				return m_id;
			}

			/**
			 * @return Media type.
			 */
			constexpr enum Type Type() const noexcept {
				return m_type;
			}

			/**
			 * @return Feature set.
			 */
			[[nodiscard]] constexpr const class Features& Features() const noexcept {
				return m_features;
			}

		private:
			AVCodecID m_id;					///< Codec id
			enum Type m_type;				///< Media type
			class Features m_features;		///< Capabilities
	};
}
