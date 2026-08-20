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
#include <StormByte/multimedia/type.hxx>

#include <optional>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Codec
	 * @brief Logical codec (id, name, description) with decoder/encoder lists.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Codec {
		public:
			/**
			 * Copy constructor.
			 */
			Codec(const Codec& other) = default;

			/**
			 * Move constructor.
			 */
			Codec(Codec&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Codec() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Codec& operator=(const Codec& other) = default;

			/**
			 * Move assignment.
			 */
			Codec& operator=(Codec&& other) = default;

			/**
			 * @return Short codec name (e.g. "h264").
			 */
			std::string Name() const noexcept;

			/**
			 * @return Long description from the backend.
			 */
			std::string Description() const noexcept;

			/**
			 * Finds a codec by short name.
			 * @param name Codec name.
			 * @return Codec or CodecNotFound.
			 */
			static ExpectedCodec Find(const std::string& name) noexcept;

			/**
			 * Finds a codec by AVCodecID-compatible id.
			 * @param id Codec id.
			 * @return Codec or CodecNotFound.
			 */
			static ExpectedCodec Find(int id) noexcept;

			/**
			 * Finds a codec by media type and optional required features (registry).
			 * @param type Media type.
			 * @param features Optional required feature set.
			 * @return Codec or CodecNotFound.
			 */
			static ExpectedCodec Find(Type type, const std::optional<Features>& features = std::nullopt) noexcept;

			/**
			 * @return Available decoders for this codec id.
			 */
			Decoders Decoders() const noexcept;

			/**
			 * @return Available encoders for this codec id.
			 */
			Encoders Encoders() const noexcept;

		private:
			int m_codec_id;						///< Backend codec id
			std::string m_name;					///< Short name
			std::string m_description;			///< Long description

			/**
			 * @param codec_id Backend codec id.
			 * @param name Short name.
			 * @param description Long description.
			 */
			Codec(int codec_id, const std::string& name, const std::string& description) noexcept;
	};
}
