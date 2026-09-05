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

#include <StormByte/multimedia/engine/codec.hxx>
#include <StormByte/multimedia/context/audio.hxx>
#include <StormByte/multimedia/context/video.hxx>
#include <StormByte/multimedia/metadata.hxx>
#include <StormByte/multimedia/type.hxx>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	/**
	 * @class Stream
	 * @brief One media stream: codec, type, metadata and optional context.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Stream {
		public:
			/**
			 * @param codec Stream codec.
			 * @param type Stream type (may be adjusted for mjpeg → Attachment).
			 */
			Stream(const Codec& codec, enum Type type) noexcept;

			/**
			 * Copy constructor.
			 */
			Stream(const Stream& other) = default;

			/**
			 * Move constructor.
			 */
			Stream(Stream&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Stream() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Stream& operator=(const Stream& other) = default;

			/**
			 * Move assignment.
			 */
			Stream& operator=(Stream&& other) = default;

			/**
			 * @return Stream type.
			 */
			enum Type Type() const noexcept;

			/**
			 * @return Stream metadata tags.
			 */
			const StormByte::Multimedia::Metadata& Metadata() const noexcept;

			/**
			 * Replaces stream metadata.
			 * @param metadata New tags.
			 */
			void Metadata(class Metadata&& metadata) noexcept;

			/**
			 * @return Optional typed context (audio/video), or null.
			 */
			std::shared_ptr<const Context::Generic> Context() const noexcept;

			/**
			 * Sets stream context (takes ownership via Move()).
			 * @param context Context object.
			 */
			void Context(Context::Generic&& context) noexcept;

			/**
			 * @return Stream codec.
			 */
			const class Codec& Codec() const noexcept;

		private:
			class Codec m_codec;									///< Codec
			enum Type m_type;										///< Type
			class Metadata m_metadata;								///< Tags
			std::shared_ptr<Context::Generic> m_context;			///< Optional context
	};
}
