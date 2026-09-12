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
#include <StormByte/logger/log.hxx>
#include <StormByte/multimedia/features.hxx>
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	class Decoder;
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
	class Frame;
	class Route;

	/**
	 * @enum DecoderFlag
	 * @brief Decoder behaviour bits. Empty mask is passthrough.
	 */
	enum class DecoderFlag: std::uint8_t {
		HeuristicsHDR10 = 1u << 0
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
	 */
	inline const DecoderFlags Heuristics{DecoderFlag::HeuristicsHDR10};

	/**
	 * @brief Binds one origin track of @p demuxer to @p decoder.
	 * @param demuxer Origin demuxer.
	 * @param decoder Destination decoder.
	 * @return @p decoder.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Decoder& operator>>(Demuxer& demuxer, Decoder& decoder) noexcept;

	/**
	 * @class Decoder
	 * @brief Decodes packets of one origin track into frames.
	 *
	 * Notice: opened implementation, once. LowLevel unit lines use
	 * @ref Step::Sparse / @ref Step::MaybeThrottle keyed by origin
	 * track. One incoming packet counts once: its frames share that
	 * sample so 1 packet → N frames does not burn the window N times.
	 * Step::Pump times each Work; Finish dumps min/max at Debug.
	 *
	 * Lineage: @ref StampLineage copies the last accepted packet
	 * Serial onto each produced frame and advances Part. Dts of
	 * that packet is copied onto @ref Frame::Dts. That is pipe
	 * lineage, not a decoded-frame count.
	 *
	 * @ref Label is `Decoder(<implementation>)` after Open pins a
	 * table row, otherwise `Decoder(t=<origin index>)`.
	 *
	 * Encode-look mode is not a public constructor. Route builds it
	 * when an Analytics filter needs a post-encode recon. That
	 * decoder has no Demuxer: it opens from the first Packet's
	 * codec parameters (the same 3-arg AVDecoder::Open used without
	 * a format context). Frames it emits carry
	 * @ref Producer::Encoder so Analytics can pair them with the
	 * origin decode look. The public ctor never enters this mode.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Decoder final: public Step {
		friend class Backend::Pipeline::Decoder;
		friend class Demuxer;
		friend class Route;
		friend Decoder& operator>>(Demuxer& demuxer, Decoder& decoder) noexcept;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Decoder for one origin track.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param track Origin stream index.
			 * @param flags Heuristics / future bits. Empty = passthrough.
			 *
			 * Origin mode. Frames carry @ref Producer::Decoder.
			 * Open waits for demuxer >> decoder.
			 */
			explicit Decoder(std::shared_ptr<StormByte::Logger::Log> log,
				int track, DecoderFlags flags = DecoderFlags{}) noexcept;

			Decoder(const Decoder& other) = delete;
			Decoder(Decoder&& other) noexcept = delete;
			~Decoder() noexcept override;
			Decoder& operator=(const Decoder& other) = delete;
			Decoder& operator=(Decoder&& other) noexcept = delete;

			/**
			 * @brief true if open and not failed.
			 * @return Open and not Failed.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Track
			 * @{
			 */

			/**
			 * @brief Bound origin track.
			 * @return Track.
			 */
			inline int Index() const noexcept {
				return m_index;
			}

			/**
			 * @brief Ceiling of the decoder input hopper.
			 * @return Max queued packets. Never 0.
			 */
			std::size_t InputCeiling() const noexcept override {
				return Ceiling;
			}

			/**
			 * @brief Flags.
			 * @return Mask.
			 */
			inline const DecoderFlags& Flags() const noexcept {
				return m_flags;
			}

			/**
			 * @brief Replaces flags (before demuxer >> decoder).
			 * @param flags New mask.
			 */
			inline void Flags(DecoderFlags flags) noexcept {
				m_flags = flags;
			}

			/**
			 * @brief Stream language tag copied from the origin File.
			 * @return Language, or empty.
			 */
			inline const std::optional<std::string>& Language() const noexcept {
				return m_language;
			}

			/**
			 * @brief Stream title tag copied from the origin File.
			 * @return Title, or empty.
			 */
			inline const std::optional<std::string>& Title() const noexcept {
				return m_title;
			}

			/**
			 * @}
			 */

			/**
			 * @name Implementation selection
			 * @{
			 */

			/**
			 * @brief Pinned FFmpeg decoder name, if any.
			 * @return Name, or empty before pin / auto-select.
			 */
			inline const std::optional<std::string>& Implementation() const noexcept {
				return m_implementation;
			}

			/**
			 * @brief Pins an FFmpeg decoder name (before demuxer >> decoder).
			 * @param name Table name. Empty clears the pin.
			 */
			void Implementation(std::string name) noexcept;

			/**
			 * @brief Extra required Feature bits.
			 * @return Mask.
			 */
			inline const Features& Require() const noexcept {
				return m_require;
			}

			/**
			 * @brief Replaces extra required Feature bits.
			 * @param features Bits the chosen table row must have.
			 */
			inline void Require(Features features) noexcept {
				m_require = features;
			}

			/**
			 * @brief Features of the opened implementation.
			 * @return Mask.
			 */
			inline const Features& Capabilities() const noexcept {
				return m_capabilities;
			}

			/**
			 * @}
			 */

		private:
			/**
			 * @class EncodeLook
			 * @brief Tag. Only Route constructs a look decoder.
			 */
			struct EncodeLook {};

			/**
			 * @brief Post-encode recon for Analytics.
			 * @param log Shared logger.
			 * @param track Origin stream index (same as the Encoder).
			 * @param tag Encode-look tag.
			 *
			 * Not callable from user code. Open does not wait for a
			 * Demuxer. The first Packet that carries codec parameters
			 * opens the backend. Produced frames are stamped
			 * @ref Producer::Encoder. Label stays Decoder(look).
			 */
			Decoder(std::shared_ptr<StormByte::Logger::Log> log,
				int track, EncodeLook tag) noexcept;

			using Step::Log;

			/**
			 * @brief Token after `STMM ` for this decoder.
			 * @return `Decoder(look)` in encode-look mode, otherwise
			 *         `Decoder(<implementation>)` or `Decoder(t=<index>)`.
			 */
			std::string Label() const noexcept override;

			/**
			 * @brief Origin: waits for the demuxer and opens the codec.
			 *        Look: becomes Ready; the codec opens on first Packet.
			 */
			void Open() noexcept override;

			/**
			 * @brief Decodes one packet and pushes frames to m_out.
			 * @param item Incoming packet.
			 */
			void Work(std::shared_ptr<Item> item) noexcept override;

			/**
			 * @brief Flushes the codec after input EoF.
			 */
			void Finish() noexcept override;

			/**
			 * @brief Pins the opened backend.
			 * @param backend Opened decode backend.
			 */
			void Bind(std::unique_ptr<Backend::Pipeline::Decoder> backend) noexcept;

			/**
			 * @brief Copies origin File tags onto this decoder.
			 * @param language Stream language, or empty.
			 * @param title Stream title, or empty.
			 */
			void Stamp(std::optional<std::string> language, std::optional<std::string> title) noexcept;

			/**
			 * @brief Records the demuxer of demuxer >> decoder.
			 * @param demuxer Origin demuxer.
			 */
			void AttachOrigin(Demuxer& demuxer) noexcept;

			/**
			 * @brief Binds a decoded handle and copies stream tags onto @p frame.
			 * @param frame Public unit.
			 * @param backend Holder of the FFmpeg frame. May be empty.
			 */
			void Attach(Frame& frame, std::unique_ptr<Backend::Pipeline::Frame> backend) noexcept;

			/**
			 * @brief Closes the duration of a subtitle cue on @p frame.
			 * @param frame Public unit produced by this decoder.
			 * @param duration Cue length on the stream clock.
			 */
			void CloseCue(Frame& frame, Property::Duration duration) noexcept;

			/**
			 * @brief Copies pipe lineage and packet Dts onto @p frame.
			 * @param frame Public unit produced by this decoder.
			 */
			void StampLineage(Frame& frame) noexcept;

			/**
			 * @brief Opens the look backend from a Packet's codecpar.
			 * @param packet First encode Packet that carries parameters.
			 * @return true if the backend is open.
			 */
			bool OpenLook(const Packet& packet) noexcept;

			/**
			 * @brief Stamps Producer::Encoder on a look frame.
			 * @param frame Frame just received from the backend.
			 */
			void StampLook(Frame& frame) noexcept;

			static constexpr std::size_t Ceiling = 32;
			int m_index;
			DecoderFlags m_flags;
			std::optional<std::string> m_language;
			std::optional<std::string> m_title;
			std::optional<std::string> m_implementation;
			Features m_require;
			Features m_capabilities;
			Demuxer* m_origin = nullptr;
			std::unique_ptr<Backend::Pipeline::Decoder> m_backend;
			std::optional<std::uint64_t> m_serial;
			std::uint64_t m_part;
			std::optional<Property::Duration> m_inDts;
			bool m_look = false;	///< Route encode-look; frames stamp Encoder
	};
}
