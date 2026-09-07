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
#include <StormByte/multimedia/features.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Encoder;
	class Mux;

	/**
	 * @namespace Engine
	 * @brief Private backends behind the public pipeline types.
	 *
	 * @ingroup multimedia_pipeline
	 */
	namespace Engine {
		/**
		 * @namespace Encoder
		 * @brief Encode backends selected by Codec::Type().
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Encoder {
			class Engine;
			/**
			 * @namespace Open
			 * @brief Shared FFmpeg open + packet wrap.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Open {
				struct Access;
			}
			/**
			 * @namespace Details
			 * @brief Per-media encode engines.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Details {
				class Video;
				class Audio;
				class Subtitle;
			}
		}
		/**
		 * @namespace Mux
		 * @brief Mux backends.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Mux {
			/**
			 * @namespace Details
			 * @brief Container and attachment mux engines.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Details {
				class Container;
			}
		}
	}

	/**
	 * @brief Sends @p frame to @p encoder. Never throws.
	 * @param frame Decoded frame (HDR metadata lives here).
	 * @param encoder Destination.
	 * @return @p frame.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Frame& operator>>(Frame& frame, Encoder& encoder) noexcept;

	/**
	 * @brief Receives one encoded packet. Never throws.
	 * @param encoder Source.
	 * @param packet Replaced on success.
	 * @return @p encoder.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Encoder& operator>>(Encoder& encoder, Packet& packet) noexcept;

	/**
	 * @class Encoder
	 * @brief Public entry point: encodes Frame into Packet for one output track.
	 *
	 * Index() is the mux output index. Copy does not use Encoder.
	 * The ctor picks Details::Video, Details::Audio or Details::Subtitle from Codec::Type().
	 * Open is lazy on the first frame >> encoder.
	 * Language() and Title() are stamped from the first frame.
	 * EncoderTag() overwrites stream metadata ENCODER on every encode.
	 * Flush() signals EOF and drains the private engine.
	 * Fail(), MuxBindStream() and MuxTakePacket() stay private.
	 * Engine::Mux::Details::Container is a friend for the header/flush path.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Encoder {
		public:
			/**
			 * @name Lifetime
			 * @{
			 */

			/**
			 * @brief Encoder for output track @p output_index and destination @p codec.
			 * @param output_index Mux track index.
			 * @param codec Registry codec. Must HasAccess(Write) at open.
			 */
			Encoder(int output_index, const Codec& codec) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Encoder(const Encoder&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source encoder.
			 */
			Encoder(Encoder&& other) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Encoder() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Encoder& operator=(const Encoder&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source encoder.
			 * @return *this.
			 */
			Encoder& operator=(Encoder&& other) noexcept;

			/** @} */

			/**
			 * @name State
			 * @{
			 */

			/**
			 * @brief true if the engine is open and not failed.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @brief Mux output track index.
			 * @return Index set at construction.
			 */
			int Index() const noexcept;

			/**
			 * @brief Destination codec.
			 * @return Registry codec bound at construction.
			 */
			const Codec& Destination() const noexcept;

			/**
			 * @brief Whether a hard error occurred.
			 * @return true on open/encode error.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text, if Failed().
			 * @return Message, or empty.
			 */
			const std::optional<std::string>& Error() const noexcept;

			/** @} */

			/**
			 * @name Stream tags
			 * @{
			 */

			/**
			 * @brief Stream language tag stamped from the decoded frame.
			 * @return Language, or empty if none was received.
			 */
			const std::optional<std::string>& Language() const noexcept;

			/**
			 * @brief Sets the stream language tag (before or after open).
			 * @param language ISO code (`spa`, `eng`, `es`, …). Empty clears it.
			 */
			void Language(std::string language) noexcept;

			/**
			 * @brief Stream title tag stamped from the decoded frame.
			 * @return Title, or empty if none was received.
			 */
			const std::optional<std::string>& Title() const noexcept;

			/**
			 * @brief Sets the stream title tag (before or after open).
			 * @param title Title from File metadata. Empty clears it.
			 */
			void Title(std::string title) noexcept;

			/**
			 * @brief Writing-application tag stamped on every encoded stream.
			 *
			 * Always `StormByte-Multimedia <version>`. Copy tracks do not
			 * use Encoder, so they keep the source tag.
			 * @return Tag string. Never empty after construction.
			 */
			const std::string& EncoderTag() const noexcept;

			/** @} */

			/**
			 * @name Implementation
			 * @{
			 */

			/**
			 * @brief Pinned FFmpeg encoder name, if any.
			 * @return Name, or empty.
			 */
			const std::optional<std::string>& Implementation() const noexcept;

			/**
			 * @brief Pins an FFmpeg encoder name. Empty clears the pin.
			 * @param name avcodec_find_encoder_by_name key.
			 */
			void Implementation(std::string name) noexcept;

			/**
			 * @brief Extra features the caller demands besides Frame HDR.
			 * @return Mask.
			 */
			const Features& Require() const noexcept;

			/**
			 * @brief Replaces the extra feature mask (before open).
			 * @param features Required bits.
			 */
			void Require(Features features) noexcept;

			/**
			 * @brief Features of the row selected at open. Empty if fallback.
			 * @return Mask.
			 */
			const Features& Capabilities() const noexcept;

			/** @} */

			/**
			 * @name Rate and style
			 * @{
			 */

			/**
			 * @brief Constant quality (CRF/CQ). Incompatible with BitRate.
			 * @param value Encoder quality value.
			 */
			void CRF(int value) noexcept;

			/**
			 * @brief CRF/CQ, if set.
			 * @return Value, or empty.
			 */
			const std::optional<int>& CRF() const noexcept;

			/**
			 * @brief Target bitrate in bits per second. Incompatible with CRF.
			 * @param bits_per_second Bitrate.
			 */
			void BitRate(std::int64_t bits_per_second) noexcept;

			/**
			 * @brief Target bitrate, if set.
			 * @return Bits per second, or empty.
			 */
			const std::optional<std::int64_t>& BitRate() const noexcept;

			/**
			 * @brief VBV ceiling in bits per second. bufsize is derived internally.
			 * @param bits_per_second Max bitrate.
			 */
			void MaxBitRate(std::int64_t bits_per_second) noexcept;

			/**
			 * @brief VBV ceiling, if set.
			 * @return Bits per second, or empty.
			 */
			const std::optional<std::int64_t>& MaxBitRate() const noexcept;

			/**
			 * @brief Encoder preset (`medium`, `p4`, …).
			 * @param name Preset name.
			 */
			void Preset(std::string name) noexcept;

			/**
			 * @brief Preset, if set.
			 * @return Name, or empty.
			 */
			const std::optional<std::string>& Preset() const noexcept;

			/**
			 * @brief Content tune (`animation`, `film`, …).
			 * @param name Tune name.
			 */
			void Tune(std::string name) noexcept;

			/**
			 * @brief Content tune, if set.
			 * @return Name, or empty.
			 */
			const std::optional<std::string>& Tune() const noexcept;

			/** @} */

			/**
			 * @name FineTune
			 * @{
			 */

			/**
			 * @brief Vendor leftovers. Not CRF/preset/tune/bufsize.
			 * @return Key/value map.
			 */
			const std::map<std::string, std::string>& FineTune() const noexcept;

			/**
			 * @brief Replaces the vendor dict (before open).
			 * @param options Key/value pairs.
			 */
			void FineTune(std::map<std::string, std::string> options) noexcept;

			/** @} */

			/**
			 * @name Pipe
			 * @{
			 */

			/**
			 * @brief Signals EOF to the engine and drains remaining packets.
			 *
			 * Safe to call more than once. Mux::Flush() calls this on every
			 * reserved encoder.
			 */
			void Flush() noexcept;

			/** @} */

			friend Frame& operator>>(Frame& frame, Encoder& encoder) noexcept;
			friend Encoder& operator>>(Encoder& encoder, Packet& packet) noexcept;
			friend class Mux;
			friend class Engine::Mux::Details::Container;
			friend struct Engine::Encoder::Open::Access;
			friend class Engine::Encoder::Engine;
			friend class Engine::Encoder::Details::Video;
			friend class Engine::Encoder::Details::Audio;
			friend class Engine::Encoder::Details::Subtitle;

		private:
			int m_index;										///< Mux output index
			const Codec* m_codec;								///< Destination codec
			std::optional<std::string> m_implementation;		///< Pinned encoder name
			std::optional<std::string> m_language;				///< Stream language from the frame
			std::optional<std::string> m_title;					///< Stream title from the frame
			std::string m_encoderTag;							///< ENCODER tag overwritten on encode
			Features m_require;									///< Extra required features
			Features m_capabilities;							///< Selected row features
			std::optional<int> m_crf;							///< CRF/CQ
			std::optional<std::int64_t> m_bitRate;				///< Target bitrate
			std::optional<std::int64_t> m_maxBitRate;			///< VBV ceiling
			std::optional<std::string> m_preset;				///< Preset
			std::optional<std::string> m_tune;					///< Tune
			std::map<std::string, std::string> m_fineTune;		///< Vendor leftovers
			std::unique_ptr<Engine::Encoder::Engine> m_engine;	///< Video / audio / subtitle backend
			bool m_failed;										///< Hard error
			std::optional<std::string> m_error;					///< Failure text

			/**
			 * @brief Marks a hard error and drops the engine.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @brief Copies codecpar and time_base onto an AVStream for Mux.
			 * @param avStream Opaque AVStream*.
			 * @return false if the engine is not open.
			 */
			bool MuxBindStream(void* avStream) noexcept;

			/**
			 * @brief Pops one pending encoded packet for Mux::Flush.
			 * @param packet Destination.
			 * @return true if @p packet was filled.
			 */
			bool MuxTakePacket(Packet& packet) noexcept;
	};
}
