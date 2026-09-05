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

#include <StormByte/multimedia/codec.hxx>
#include <StormByte/multimedia/metadata/stream.hxx>
#include <StormByte/multimedia/property/audio.hxx>
#include <StormByte/multimedia/property/video.hxx>

#include <chrono>
#include <optional>
#include <variant>

/**
 * @namespace StormByte::Multimedia
 * @brief Public multimedia types: codecs, containers, streams and files.
 */
namespace StormByte::Multimedia {
	class File;

	/**
	 * @class Stream
	 * @brief One media stream: registry Codec, tags and optional property bag.
	 *
	 * Copies share the same Codec instance. The Codec outlives every Stream.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Stream {
		public:
			/**
			 * @brief Per-stream property bag.
			 */
			using Properties = std::variant<std::monostate, Property::Video, Property::Audio>;

			/**
			 * @brief Copy constructor.
			 */
			Stream(const Stream&) = default;

			/**
			 * @brief Move constructor.
			 */
			Stream(Stream&&) = default;

			/**
			 * @brief Destructor.
			 */
			~Stream() = default;

			/**
			 * @brief Copy assignment.
			 * @return *this.
			 */
			Stream& operator=(const Stream&) = default;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Stream& operator=(Stream&&) = default;

			/**
			 * @brief Codec of this stream.
			 * @return Registry codec.
			 */
			const class Codec& Codec() const noexcept { return m_codec; }

			/**
			 * @brief Media kind of the codec.
			 * @return Audio, Video, Subtitle, Attachment or Unknown.
			 */
			Type Type() const noexcept { return m_codec.Type(); }

			/**
			 * @brief Per-stream tags captured at Open.
			 * @return Metadata snapshot.
			 */
			const Metadata::Stream& Metadata() const noexcept { return m_metadata; }

			/**
			 * @brief Stream duration, if known.
			 * @return Duration in nanoseconds, or empty.
			 */
			const std::optional<std::chrono::nanoseconds>& Duration() const noexcept { return m_duration; }

			/**
			 * @brief Video property bag, if this stream has one.
			 * @return Video properties, or nullptr.
			 */
			const Property::Video* Video() const noexcept {
				return std::get_if<Property::Video>(&m_properties);
			}

			/**
			 * @brief Audio property bag, if this stream has one.
			 * @return Audio properties, or nullptr.
			 */
			const Property::Audio* Audio() const noexcept {
				return std::get_if<Property::Audio>(&m_properties);
			}

		private:
			friend class File;

			const class Codec& m_codec;							///< Registry codec
			Metadata::Stream m_metadata;						///< Stream tags
			std::optional<std::chrono::nanoseconds> m_duration;	///< Stream duration
			Properties m_properties;							///< Video, audio, or none

			/**
			 * @brief File-only constructor.
			 * @param codec Registry codec.
			 * @param metadata Stream tags.
			 * @param duration Stream duration, if known.
			 * @param properties Video or audio bag, or monostate.
			 */
			Stream(const class Codec& codec, Metadata::Stream metadata,
				std::optional<std::chrono::nanoseconds> duration = std::nullopt,
				Properties properties = std::monostate {}) noexcept
			: m_codec(codec), m_metadata(std::move(metadata)),
			m_duration(duration), m_properties(std::move(properties)) {}
	};
}
