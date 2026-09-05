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

#include <StormByte/multimedia/engine/typedefs.hxx>
#include <StormByte/multimedia/features.hxx>

#include <string>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Encoder
	 * @brief Concrete encoder implementation (name + feature set).
	 *
	 * Constructed only via Codec::Encoders().
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Encoder final {
		friend class Codec;
	public:
		/**
		 * Copy constructor.
		 */
		Encoder(const Encoder& other) = default;

		/**
		 * Move constructor.
		 */
		Encoder(Encoder&& other) noexcept = default;

		/**
		 * Destructor.
		 */
		~Encoder() noexcept = default;

		/**
		 * Copy assignment.
		 */
		Encoder& operator=(const Encoder& other) = default;

		/**
		 * Move assignment.
		 */
		Encoder& operator=(Encoder&& other) noexcept = default;

		/**
		 * @return Underlying codec id.
		 */
		int CodecID() const noexcept;

		/**
		 * @return Detected capability flags.
		 */
		const StormByte::Multimedia::Features& Features() const noexcept;

		/**
		 * @return Implementation name (e.g. "libx265").
		 */
		const std::string& Name() const noexcept;

	private:
		int m_id;										///< Codec id
		std::string m_name;								///< Implementation name
		StormByte::Multimedia::Features m_features;		///< Capabilities

		/**
		 * @param id Codec id.
		 * @param name Implementation name.
		 */
		Encoder(int id, const std::string& name) noexcept;

		/**
		 * @param id Codec id.
		 * @param name Implementation name.
		 */
		Encoder(int id, std::string&& name) noexcept;

		/**
		 * Merges registry + FFmpeg capability bits for @p name.
		 * @param name Implementation name.
		 * @return Feature set.
		 */
		static StormByte::Multimedia::Features DetectFeatures(const std::string_view& name) noexcept;
	};
}
