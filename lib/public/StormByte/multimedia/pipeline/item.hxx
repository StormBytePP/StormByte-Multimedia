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

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Decoder;		///< Packet-to-frame decode step.
	class Demuxer;		///< Reads packets from the input file.
	class Encoder;		///< Frame-to-packet encode step.
	class Frame;		///< Decoded access unit.
	class Muxer;		///< Writes packets to the output file.
	class Packet;		///< Compressed access unit.

	/**
	 * @namespace Filter
	 * @brief Frame and packet filters attached to a route.
	 *
	 * Forward-declared so @ref Item can friend
	 * @ref Filter::FFmpeg without including the generated filter header.
	 */
	namespace Filter {
		class FFmpeg;	///< Filter base. Not a leaf: inherit Process, Packet or Analytics.
	}

	/**
	 * @namespace Engine
	 * @brief Private backends. Public headers only forward-declare them.
	 *
	 * @ingroup multimedia_pipeline
	 */
	namespace Engine {
		/**
		 * @namespace Frame
		 * @brief Decoded-frame backend.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Frame {
			class Engine;	///< Opaque holder of the backend @c AVFrame.
		}
		/**
		 * @namespace Packet
		 * @brief Compressed-AU backend.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Packet {
			class Engine;	///< Opaque holder of the backend @c AVPacket.
		}
		/**
		 * @namespace Encoder
		 * @brief Encode backends.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Encoder {
			/**
			 * @namespace Open
			 * @brief Shared encoder open helper.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Open {
				struct Access;	///< Grants Encoder::Open access to unit internals.
			}
			/**
			 * @namespace Details
			 * @brief Per-media encode engines.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Details {
				class Video;		///< Video encoder engine.
				class Audio;		///< Audio encoder engine.
				class Subtitle;		///< Subtitle encoder engine.
			}
		}
		/**
		 * @namespace Decoder
		 * @brief Decode backends.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Decoder {
			/**
			 * @namespace Details
			 * @brief Per-media decode engines.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Details {
				class Video;		///< Video decoder engine.
				class Audio;		///< Audio decoder engine.
				class Subtitle;		///< Subtitle decoder engine.
			}
		}
	}

	/**
	 * @enum Producer
	 * @brief Step that created this @ref Item.
	 *
	 * Set in the private constructor. Only a getter afterwards.
	 * Passthrough and filter @c Save do not stamp a new producer.
	 * @ref Remuxer forwards the origin producer.
	 */
	enum class Producer: std::uint8_t {
		Decoder,	///< @ref Decoder
		Demuxer,	///< @ref Demuxer
		Encoder,	///< @ref Encoder
		Muxer		///< @ref Muxer
	};

	/**
	 * @brief Converts a @ref Producer to a string literal.
	 * @param producer Value to convert.
	 * @return Null-terminated name, or `"Invalid"`.
	 */
	constexpr const char* ToString(Producer producer) noexcept {
		switch (producer) {
			case Producer::Decoder:		return "Decoder";
			case Producer::Demuxer:		return "Demuxer";
			case Producer::Encoder:		return "Encoder";
			case Producer::Muxer:		return "Muxer";
			default:					return "Invalid";
		}
	}

	/**
	 * @class Item
	 * @brief Common face of a pipeline unit.
	 *
	 * @ref Frame and @ref Packet both inherit it, so a filter, a route
	 * or a sink can hold one pointer and still know @ref Kind, media
	 * @ref Type, origin @ref Track and @ref Producer.
	 *
	 * @see Frame
	 * @see Packet
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Item {
		friend class Decoder;
		friend class Demuxer;
		friend class Encoder;
		friend class Filter::FFmpeg;
		friend class Frame;
		friend class Muxer;
		friend class Packet;
		friend struct Engine::Encoder::Open::Access;
		friend class Engine::Encoder::Details::Video;
		friend class Engine::Encoder::Details::Audio;
		friend class Engine::Encoder::Details::Subtitle;
		friend class Engine::Decoder::Details::Video;
		friend class Engine::Decoder::Details::Audio;
		friend class Engine::Decoder::Details::Subtitle;
		friend class Engine::Frame::Engine;
		friend class Engine::Packet::Engine;

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
			 * @return @ref Kind::Frame or @ref Kind::Packet.
			 */
			enum Kind Kind() const noexcept {
				return m_kind;
			}

			/**
			 * @brief Media of this access unit.
			 * @return @ref Type::Video, @ref Type::Audio or
			 *         @ref Type::Subtitle on a live unit;
			 *         @ref Type::Unknown on the empty sentinel.
			 */
			enum Type Type() const noexcept {
				return m_type;
			}

			/**
			 * @brief Origin track index.
			 * @return Input container stream index. The muxer does not
			 *         overwrite it. @c -1 on the empty sentinel.
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
			 * @name Construction
			 * @{
			 */

			/**
			 * @brief Builds the facade.
			 * @param track Origin container stream index.
			 * @param type Media of this unit.
			 * @param kind @ref Kind::Frame or @ref Kind::Packet.
			 * @param producer Step that created this unit.
			 */
			Item(int track, enum Type type, enum Kind kind, enum Producer producer) noexcept
			: m_track(track), m_type(type), m_kind(kind), m_producer(producer) {}

			/**
			 * @}
			 */

			int m_track;				///< Origin container stream index
			enum Type m_type;			///< Video / Audio / Subtitle / Unknown
			enum Kind m_kind;			///< Frame or Packet
			enum Producer m_producer;	///< Step that created this unit
	};
}
