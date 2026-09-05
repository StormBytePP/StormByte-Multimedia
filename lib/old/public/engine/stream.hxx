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
