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
 */
namespace StormByte::Multimedia::Pipeline {
	class Demux;

	/**
	 * @enum DecoderFlag
	 * @brief Decoder behaviour bits. Empty mask is passthrough.
	 */
	enum class DecoderFlag: std::uint8_t {
		HeuristicsHDR10 = 1u << 0	///< Fill HDR10::DEFAULT when colorimetry is HDR10 without MDM
		// HeuristicsAudio  = 1u << 1	///< Future audio sanitising
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
	 * @brief All current heuristic bits. Extend this value when adding flags.
	 */
	inline const DecoderFlags Heuristics{DecoderFlag::HeuristicsHDR10};

	class Decoder;

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

	/**
	 * @class Decoder
	 * @brief Decodes packets of one demuxed stream into Frame.
	 *
	 * Construct with a stream index. demux >> decoder opens the backend
	 * on the Demux context. packet >> decoder ignores other indexes.
	 * decoder >> frame is a no-op on TryAgain. After the demuxer hits
	 * EOF, Flush() then drain with decoder >> frame until StreamIndex()
	 * is -1. Failbit on open/decode errors. Copy is not a Decoder mode.
	 *
	 * Frame steps (bundled or user) go in Pipe(). They run inside
	 * decoder >> frame. An empty pipe is identity.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Decoder {
		public:
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
			 */
			Decoder(Decoder&&) noexcept;

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
			 * @return *this.
			 */
			Decoder& operator=(Decoder&&) noexcept;

			/**
			 * @brief true if open and not failed.
			 */
			explicit operator bool() const noexcept;

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
			 * @brief Frame filter pipe. Add bundled or custom steps before reading frames.
			 * @return Pipe.
			 */
			Filter::FramePipe& Pipe() noexcept;

			/**
			 * @brief Frame filter pipe.
			 * @return Pipe.
			 */
			const Filter::FramePipe& Pipe() const noexcept;

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

			friend Decoder& operator>>(Demux& demux, Decoder& decoder) noexcept;
			friend Packet& operator>>(Packet& packet, Decoder& decoder) noexcept;
			friend Decoder& operator>>(Decoder& decoder, Frame& frame) noexcept;

		private:
			class Impl;

			/* bind */
			int m_index;							///< Stream index
			std::unique_ptr<Impl> m_impl;			///< Opened backend

			/* flags */
			DecoderFlags m_flags;					///< Heuristics / future bits

			/* filters */
			Filter::FramePipe m_pipe;				///< Frame steps

			/* fail */
			bool m_failed;							///< Hard error
			std::optional<std::string> m_error;		///< Failure text

			/**
			 * @brief Marks a hard error.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @brief Adopts backend state built by demux >> decoder.
			 * @param impl Opened implementation.
			 */
			void Bind(std::unique_ptr<Impl> impl) noexcept;
	};
}
