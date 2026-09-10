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

#include <StormByte/multimedia/type.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Demux;		///< Reads packets from the input file.
	class Mux;			///< Writes packets to the output file.
	class Decoder;		///< Packet-to-frame decode step.
	class Encoder;		///< Frame-to-packet encode step.
	class Frame;		///< Decoded access unit.
	class Packet;		///< Compressed access unit.

	/**
	 * @namespace Filter
	 * @brief Frame and packet filters attached to a route.
	 *
	 * Forward-declared so @ref Item can friend
	 * @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg
	 * without including the generated filter header.
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
	 * @enum Kind
	 * @brief Whether an @ref Item is a decoded frame or a compressed packet.
	 *
	 * Distinct from @ref StormByte::Multimedia::Type (Video / Audio / Subtitle).
	 * Used as the static @c Accepts mask on filters. Values are bit flags:
	 * @c 0 is not a valid kind. Combine with @ref StormByte::Bitmask /
	 * @ref Filter::Accepts. @ref ToString understands a single flag only.
	 */
	enum class Kind: std::uint8_t {
		Packet = 1 << 0,	///< @ref StormByte::Multimedia::Pipeline::Packet
		Frame  = 1 << 1		///< @ref StormByte::Multimedia::Pipeline::Frame
	};

	/**
	 * @brief Converts a single @ref Kind flag to a string literal.
	 * @param kind Value to convert.
	 * @return `"Frame"`, `"Packet"`, or `"Invalid"` for a mask or zero.
	 */
	constexpr const char* ToString(Kind kind) noexcept {
		switch (kind) {
			case Kind::Frame:	return "Frame";		///< Frame
			case Kind::Packet:	return "Packet";	///< Packet
			default:			return "Invalid";	///< Combined mask or zero
		}
	}

	/**
	 * @enum Producer
	 * @brief Step that created this @ref Item.
	 *
	 * Set in the private constructor. Only a getter afterwards.
	 * Passthrough and filter @c Save do not stamp a new producer.
	 */
	enum class Producer: std::uint8_t {
		Demux,		///< @ref StormByte::Multimedia::Pipeline::Demux
		Decoder,	///< @ref StormByte::Multimedia::Pipeline::Decoder
		Encoder,	///< @ref StormByte::Multimedia::Pipeline::Encoder
		Mux			///< @ref StormByte::Multimedia::Pipeline::Mux
	};

	/**
	 * @brief Converts a @ref Producer to a string literal.
	 * @param producer Value to convert.
	 * @return Null-terminated name, or `"Invalid"`.
	 */
	constexpr const char* ToString(Producer producer) noexcept {
		switch (producer) {
			case Producer::Demux:		return "Demux";		///< Demux
			case Producer::Decoder:		return "Decoder";	///< Decoder
			case Producer::Encoder:		return "Encoder";	///< Encoder
			case Producer::Mux:			return "Mux";		///< Mux
			default:					return "Invalid";	///< Out of range
		}
	}

	/**
	 * @class Item
	 * @brief Facade shared by @ref Frame and @ref Packet.
	 *
	 * Carries @ref Kind, @ref StormByte::Multimedia::Type, origin track
	 * and @ref Producer. Construction is private: only the engines and
	 * steps that produce units (and the two derived types) may build one.
	 * The muxer does not overwrite the origin track: @ref Track stays the
	 * input stream index for the life of the unit.
	 *
	 * The tube stores @c std::shared_ptr of the derived type through this
	 * facade. There is no public clone.
	 *
	 * @see StormByte::Multimedia::Pipeline::Frame
	 * @see StormByte::Multimedia::Pipeline::Packet
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Item {
		friend class Frame;
		friend class Packet;
		friend class Demux;
		friend class Mux;
		friend class Decoder;
		friend class Encoder;
		friend class Filter::FFmpeg;
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
			Kind Kind() const noexcept {
				return m_kind;
			}

			/**
			 * @brief Media of this access unit.
			 * @return @ref StormByte::Multimedia::Type::Video,
			 *         @ref StormByte::Multimedia::Type::Audio or
			 *         @ref StormByte::Multimedia::Type::Subtitle
			 *         on a live unit;
			 *         @ref StormByte::Multimedia::Type::Unknown on the
			 *         empty sentinel.
			 *
			 * Not @ref StormByte::Multimedia::Type::Copy: that value is a
			 * track mode on the job, not a kind of access unit.
			 */
			enum StormByte::Multimedia::Type Type() const noexcept {
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
			 * @param type Media of this unit. Not Copy.
			 * @param kind @ref Kind::Frame or @ref Kind::Packet.
			 * @param producer Step that created this unit.
			 */
			Item(int track, enum StormByte::Multimedia::Type type, enum Kind kind, enum Producer producer) noexcept
			: m_track(track), m_type(type), m_kind(kind), m_producer(producer) {}

			/**
			 * @}
			 */

			int m_track;									///< Origin container stream index
			enum StormByte::Multimedia::Type m_type;		///< Video / Audio / Subtitle / Unknown
			enum Kind m_kind;								///< Frame or Packet
			enum Producer m_producer;						///< Step that created this unit
	};
}
