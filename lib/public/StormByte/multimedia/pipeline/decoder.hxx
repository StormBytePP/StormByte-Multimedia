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
	class Decoder;	///< Decode backend behind @ref StormByte::Multimedia::Pipeline::Decoder.
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
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Decoder final: public Step {
		friend class Backend::Pipeline::Decoder;
		friend class Demuxer;
		friend Decoder& operator>>(Demuxer& demuxer, Decoder& decoder) noexcept;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Decoder for one origin track.
			 * @param track Origin stream index.
			 * @param flags Heuristics / future bits. Empty = passthrough.
			 */
			explicit Decoder(int track, DecoderFlags flags = DecoderFlags{}) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source decoder.
			 */
			Decoder(const Decoder& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Decoder to take.
			 */
			Decoder(Decoder&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Decoder() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source decoder.
			 * @return *this.
			 */
			Decoder& operator=(const Decoder& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Decoder to take.
			 * @return *this.
			 */
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
			 * @brief Waits for the bound demuxer, then opens the codec.
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
			 * @brief Copies pipe lineage onto @p frame and advances Part.
			 * @param frame Public unit produced by this decoder.
			 */
			void StampLineage(Frame& frame) noexcept;

			static constexpr std::size_t Ceiling = 8;							///< Input hopper ceiling
			int m_index;														///< Origin track
			DecoderFlags m_flags;												///< Heuristics
			std::optional<std::string> m_language;								///< Language tag
			std::optional<std::string> m_title;									///< Title tag
			std::optional<std::string> m_implementation;						///< Pinned decoder name
			Features m_require;													///< Extra required bits
			Features m_capabilities;											///< Opened capabilities
			Demuxer* m_origin = nullptr;										///< Bound demuxer
			std::unique_ptr<Backend::Pipeline::Decoder> m_backend;				///< Decode backend
			std::optional<std::uint64_t> m_serial;								///< Lineage of the last accepted packet
			std::uint64_t m_part;												///< Next Part inside m_serial
	};
}
