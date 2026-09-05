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

#include <StormByte/multimedia/engine/typedefs.hxx>
#include <StormByte/multimedia/features.hxx>

#include <string>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Decoder
	 * @brief Concrete decoder implementation (name + feature set).
	 *
	 * Constructed only via Codec::Decoders().
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Decoder final {
		friend class Codec;
	public:
		/**
		 * Copy constructor.
		 */
		Decoder(const Decoder& other) = default;

		/**
		 * Move constructor.
		 */
		Decoder(Decoder&& other) noexcept = default;

		/**
		 * Destructor.
		 */
		~Decoder() noexcept = default;

		/**
		 * Copy assignment.
		 */
		Decoder& operator=(const Decoder& other) = default;

		/**
		 * Move assignment.
		 */
		Decoder& operator=(Decoder&& other) noexcept = default;

		/**
		 * @return Underlying codec id.
		 */
		int CodecID() const noexcept;

		/**
		 * @return Detected capability flags.
		 */
		const StormByte::Multimedia::Features& Features() const noexcept;

		/**
		 * @return Implementation name (e.g. "hevc_nvdec").
		 */
		const std::string& Name() const noexcept;

	private:
		int m_id;										///< Codec id
		std::string m_name;								///< Implementation name
		class Features m_features;						///< Capabilities

		/**
		 * @param id Codec id.
		 * @param name Implementation name.
		 */
		Decoder(int id, const std::string& name) noexcept;

		/**
		 * @param id Codec id.
		 * @param name Implementation name.
		 */
		Decoder(int id, std::string&& name) noexcept;

		/**
		 * Merges registry + FFmpeg capability bits for @p name.
		 * @param name Implementation name.
		 * @return Feature set.
		 */
		static StormByte::Multimedia::Features DetectFeatures(const std::string_view& name) noexcept;
	};
}
