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

#include <StormByte/bitmask.hxx>
#include <StormByte/multimedia/features.hxx>
#include <StormByte/multimedia/pipeline/filters/frame_pipe.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Demux;
	class Decoder;

	/**
	 * @namespace Engine
	 * @brief Private backends. Public headers only forward-declare them.
	 *
	 * @ingroup multimedia_pipeline
	 */
	namespace Engine {
		/**
		 * @namespace Decoder
		 * @brief Decode backends selected by stream type.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Decoder {
			class Engine;
			/**
			 * @namespace Details
			 * @brief Per-media decode engines.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Details {
				class Video;
				class Audio;
				class Subtitle;
			}
		}
	}

	/**
	 * @defgroup decoder_flags Decoder flags
	 * @{
	 */

	/**
	 * @enum DecoderFlag
	 * @brief Decoder behaviour bits. Empty mask is passthrough.
	 */
	enum class DecoderFlag: std::uint8_t {
		HeuristicsHDR10 = 1u << 0	///< Fill HDR10::DEFAULT when colorimetry is HDR10 without MDM
	};

	/**
	 * @class DecoderFlags
	 * @brief Bitmask of DecoderFlag.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC DecoderFlags: public StormByte::Bitmask<DecoderFlags, DecoderFlag> {
		public:
			using Bitmask::Bitmask;
	};

	/**
	 * @brief All current heuristic bits.
	 *
	 * Extend this value when adding flags.
	 */
	inline const DecoderFlags Heuristics{DecoderFlag::HeuristicsHDR10};

	/** @} */

	/**
	 * @defgroup decoder_ops Decoder stream operators
	 * @{
	 */

	/**
	 * @brief Opens @p decoder on a stream of @p demux. Never throws.
	 * @param demux Open demuxer.
	 * @param decoder Destination.
	 * @return @p decoder.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Decoder& operator>>(Demux& demux, Decoder& decoder) noexcept;

	/**
	 * @brief Sends @p packet if its stream index matches. Never throws.
	 * @param packet Compressed packet.
	 * @param decoder Destination.
	 * @return @p packet.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Packet& operator>>(Packet& packet, Decoder& decoder) noexcept;

	/**
	 * @brief Receives one decoded frame after the frame pipe. Never throws.
	 * @param decoder Source.
	 * @param frame Replaced on success.
	 * @return @p decoder.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Decoder& operator>>(Decoder& decoder, Frame& frame) noexcept;

	/** @} */

	/**
	 * @class Decoder
	 * @brief Decodes packets of one demuxed stream into Frame.
	 *
	 * Public entry point. Work lives in Engine::Decoder::Details::{Video,Audio,Subtitle}.
	 * Construct with a stream index. Implementation() pins an FFmpeg decoder
	 * name from the handcrafted table. Empty pin picks the lowest preference
	 * row that covers Require() plus stream HDR10 / HDR10Plus. demux >> decoder
	 * opens the backend and copies stream language and title from File metadata.
	 * packet >> decoder ignores other indexes.
	 * decoder >> frame is a no-op on TryAgain. After the demuxer hits EOF,
	 * Flush() then drain with decoder >> frame until StreamIndex() is -1.
	 * Failbit on open/decode errors. Copy is not a Decoder mode.
	 *
	 * Frame steps go in Pipe(). They run inside decoder >> frame.
	 * An empty pipe is identity.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Decoder {
		public:
			/**
			 * @defgroup decoder_lifetime Lifetime
			 * @{
			 */

			/**
			 * @brief Decoder for @p stream_index. Does not open the backend.
			 * @param stream_index File stream index.
			 * @param flags Heuristics / future bits. Empty = passthrough.
			 */
			explicit Decoder(int stream_index, DecoderFlags flags = DecoderFlags{}) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Decoder(const Decoder&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Decoder to take.
			 */
			Decoder(Decoder&& other) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Decoder() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Decoder& operator=(const Decoder&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Decoder to take.
			 * @return *this.
			 */
			Decoder& operator=(Decoder&& other) noexcept;

			/**
			 * @brief true if open and not failed.
			 */
			explicit operator bool() const noexcept;

			/** @} */

			/**
			 * @defgroup decoder_bind Bind
			 * @{
			 */

			/**
			 * @brief Bound stream index.
			 * @return Index.
			 */
			int Index() const noexcept;

			/**
			 * @brief Flags.
			 * @return Mask.
			 */
			const DecoderFlags& Flags() const noexcept;

			/**
			 * @brief Replaces flags (before demux >> decoder).
			 * @param flags New mask.
			 */
			void Flags(DecoderFlags flags) noexcept;

			/**
			 * @brief Stream language tag copied from File metadata.
			 * @return Language, or empty if the stream had none.
			 */
			const std::optional<std::string>& Language() const noexcept;

			/**
			 * @brief Sets the stream language tag (used before demux >> decoder if needed).
			 * @param language ISO code (`spa`, `eng`, `es`, …). Empty clears it.
			 */
			void Language(std::string language) noexcept;

			/**
			 * @brief Stream title tag copied from File metadata.
			 * @return Title, or empty if the stream had none.
			 */
			const std::optional<std::string>& Title() const noexcept;

			/**
			 * @brief Sets the stream title tag (used before demux >> decoder if needed).
			 * @param title Title from File metadata. Empty clears it.
			 */
			void Title(std::string title) noexcept;

			/** @} */

			/**
			 * @defgroup decoder_impl Implementation selection
			 * @{
			 */

			/**
			 * @brief Pinned FFmpeg decoder name, if any.
			 * @return Name, or empty before pin / auto-select.
			 */
			const std::optional<std::string>& Implementation() const noexcept;

			/**
			 * @brief Pins an FFmpeg decoder name (before demux >> decoder).
			 * @param name Table `name` (`hevc`, `hevc_cuvid`, …). Empty clears the pin.
			 */
			void Implementation(std::string name) noexcept;

			/**
			 * @brief Extra required Feature bits (before demux >> decoder).
			 * @return Mask. Empty = only stream-derived HDR bits.
			 */
			const Features& Require() const noexcept;

			/**
			 * @brief Replaces extra required Feature bits (before demux >> decoder).
			 * @param features Bits the chosen table row must have.
			 */
			void Require(Features features) noexcept;

			/**
			 * @brief Features of the opened implementation.
			 * @return Mask. Empty if not open or fallback without a table row.
			 */
			const Features& Capabilities() const noexcept;

			/** @} */

			/**
			 * @defgroup decoder_pipe Frame pipe
			 * @{
			 */

			/**
			 * @brief Frame filter pipe. Add bundled or custom steps before reading frames.
			 * @return Pipe.
			 */
			Filter::FramePipe& Pipe() noexcept;

			/**
			 * @brief Frame filter pipe.
			 * @return Pipe.
			 */
			const Filter::FramePipe& Pipe() const noexcept;

			/** @} */

			/**
			 * @defgroup decoder_error Failure
			 * @{
			 */

			/**
			 * @brief Whether a hard error occurred.
			 * @return true on open/decode error.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text, if Failed().
			 * @return Message, or empty.
			 */
			const std::optional<std::string>& Error() const noexcept;

			/**
			 * @brief Signals EOF to the backend. Drain with decoder >> frame afterwards.
			 *
			 * Required after the demuxer reaches EOF when frame threading
			 * is enabled; otherwise delayed frames stay inside libavcodec.
			 */
			void Flush() noexcept;

			/**
			 * @brief Marks a hard error.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			/** @} */

			friend Decoder& operator>>(Demux& demux, Decoder& decoder) noexcept;
			friend Packet& operator>>(Packet& packet, Decoder& decoder) noexcept;
			friend Decoder& operator>>(Decoder& decoder, Frame& frame) noexcept;
			friend class Engine::Decoder::Details::Video;
			friend class Engine::Decoder::Details::Audio;
			friend class Engine::Decoder::Details::Subtitle;

		private:
			int m_index;												///< Stream index
			std::unique_ptr<Engine::Decoder::Engine> m_engine;			///< Opened backend
			DecoderFlags m_flags;										///< Heuristics / future bits
			std::optional<std::string> m_implementation;				///< Pinned or selected FFmpeg name
			std::optional<std::string> m_language;						///< Stream language from File metadata
			std::optional<std::string> m_title;							///< Stream title from File metadata
			Features m_require;											///< Extra required bits
			Features m_capabilities;									///< Features of the opened row
			Filter::FramePipe m_pipe;									///< Frame steps
			bool m_failed;												///< Hard error
			std::optional<std::string> m_error;							///< Failure text

			/**
			 * @brief Adopts backend state built by demux >> decoder.
			 * @param engine Opened implementation.
			 */
			void Bind(std::unique_ptr<Engine::Decoder::Engine> engine) noexcept;
	};
}
