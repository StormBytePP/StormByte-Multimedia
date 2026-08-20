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
