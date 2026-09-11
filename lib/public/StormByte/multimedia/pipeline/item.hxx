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

#include <StormByte/multimedia/pipeline/typedefs.hxx>
#include <StormByte/multimedia/type.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	class Frame;	///< Decoded-AU holder behind @ref StormByte::Multimedia::Pipeline::Frame.
	class Packet;	///< Compressed-AU holder behind @ref StormByte::Multimedia::Pipeline::Packet.
}

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Decoder;
	class Demuxer;
	class Encoder;
	class Frame;
	class Muxer;
	class Packet;

	/**
	 * @namespace Filter
	 * @brief Frame and packet filters attached to a route.
	 *
	 * @ingroup multimedia_pipeline
	 */
	namespace Filter {
		class FFmpeg;	///< Filter base. Not a leaf: inherit Process, Packet or Analytics.
	}

	/**
	 * @enum Producer
	 * @brief Step that created this Item.
	 *
	 * @ingroup multimedia_pipeline
	 */
	enum class Producer: std::uint8_t {
		Decoder,	///< Decoder
		Demuxer,	///< Demuxer
		Encoder,	///< Encoder
		Muxer		///< Muxer
	};

	/**
	 * @brief Converts a Producer to a string literal.
	 * @param producer Value to convert.
	 * @return Null-terminated name, or "Invalid".
	 */
	constexpr const char* ToString(Producer producer) noexcept {
		switch (producer) {
			case Producer::Decoder:	return "Decoder";
			case Producer::Demuxer:	return "Demuxer";
			case Producer::Encoder:	return "Encoder";
			case Producer::Muxer:	return "Muxer";
			default:				return "Invalid";
		}
	}

	/**
	 * @class Item
	 * @brief Facade shared by Frame and Packet.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Item {
		friend class Backend::Pipeline::Frame;
		friend class Backend::Pipeline::Packet;
		friend class Decoder;
		friend class Demuxer;
		friend class Encoder;
		friend class Filter::FFmpeg;
		friend class Frame;
		friend class Muxer;
		friend class Packet;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Destructor.
			 */
			virtual ~Item() noexcept = default;

			/**
			 * @}
			 */

			/**
			 * @name Identity
			 * @{
			 */

			/**
			 * @brief Whether this unit is a frame or a packet.
			 * @return Kind::Frame or Kind::Packet.
			 */
			Kind Kind() const noexcept {
				return m_kind;
			}

			/**
			 * @brief Media of this access unit.
			 * @return Video, Audio, Subtitle, or Unknown on the empty sentinel.
			 */
			enum StormByte::Multimedia::Type Type() const noexcept {
				return m_type;
			}

			/**
			 * @brief Origin track index.
			 * @return Input container stream index. -1 on the empty sentinel.
			 */
			int Track() const noexcept {
				return m_track;
			}

			/**
			 * @brief Step that created this unit.
			 * @return Value passed to the private constructor.
			 */
			enum Producer Producer() const noexcept {
				return m_producer;
			}

			/**
			 * @}
			 */

		protected:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Copy constructor.
			 * @param other Source item.
			 */
			Item(const Item& other) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Item to take.
			 */
			Item(Item&& other) noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @param other Source item.
			 * @return *this.
			 */
			Item& operator=(const Item& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Item to take.
			 * @return *this.
			 */
			Item& operator=(Item&& other) noexcept = default;

			/**
			 * @}
			 */

		private:
			/**
			 * @brief Builds the facade.
			 * @param track Origin container stream index.
			 * @param type Media of this unit. Not Copy.
			 * @param kind Kind::Frame or Kind::Packet.
			 * @param producer Step that created this unit.
			 */
			Item(int track, enum StormByte::Multimedia::Type type, enum Kind kind, enum Producer producer) noexcept
			: m_track(track), m_type(type), m_kind(kind), m_producer(producer) {}

			int m_track;									///< Origin container stream index
			enum StormByte::Multimedia::Type m_type;		///< Video / Audio / Subtitle / Unknown
			enum Kind m_kind;								///< Frame or Packet
			enum Producer m_producer;						///< Step that created this unit
	};
}
