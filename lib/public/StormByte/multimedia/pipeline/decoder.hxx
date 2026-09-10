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
	class Transcode;

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
	 * @brief Opens @p decoder on a stream of @p demux. Never throws.
	 * @param demux Open demuxer.
	 * @param decoder Destination.
	 * @return @p decoder.
	 *
	 * Attaches the backend and launches the decoder worker.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Decoder& operator>>(Demux& demux, Decoder& decoder) noexcept;

	/**
	 * @class Decoder
	 * @brief Decodes packets of one origin track into frames.
	 *
	 * A @ref Step, @c final. The constructor does not start the worker.
	 * @c demux >> decoder attaches the backend and launches.
	 * @ref Work sends one packet and drains frames to @ref m_out.
	 * @ref Finish flushes the codec. Errors are @ref Step::Fail.
	 *
	 * @ref Item::Track on outgoing frames stays the origin index.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Decoder final: public Step {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Decoder for one origin track. Does not launch.
			 * @param track Origin stream index.
			 * @param flags Heuristics / future bits. Empty = passthrough.
			 *
			 * The backend is attached later by @c demux >> decoder.
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
			 * @return Open and not @ref Failed.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Bind
			 * @{
			 */

			/**
			 * @brief Bound origin track.
			 * @return Track.
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
			 * @brief Stream language tag.
			 * @return Language, or empty.
			 */
			const std::optional<std::string>& Language() const noexcept;

			/**
			 * @brief Sets the stream language tag.
			 * @param language ISO code. Empty clears it.
			 */
			void Language(std::string language) noexcept;

			/**
			 * @brief Stream title tag.
			 * @return Title, or empty.
			 */
			const std::optional<std::string>& Title() const noexcept;

			/**
			 * @brief Sets the stream title tag.
			 * @param title Title. Empty clears it.
			 */
			void Title(std::string title) noexcept;

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
			const std::optional<std::string>& Implementation() const noexcept;

			/**
			 * @brief Pins an FFmpeg decoder name (before demux >> decoder).
			 * @param name Table name. Empty clears the pin.
			 */
			void Implementation(std::string name) noexcept;

			/**
			 * @brief Extra required Feature bits.
			 * @return Mask.
			 */
			const Features& Require() const noexcept;

			/**
			 * @brief Replaces extra required Feature bits.
			 * @param features Bits the chosen table row must have.
			 */
			void Require(Features features) noexcept;

			/**
			 * @brief Features of the opened implementation.
			 * @return Mask.
			 */
			const Features& Capabilities() const noexcept;

			/**
			 * @}
			 */

			friend Decoder& operator>>(Demux& demux, Decoder& decoder) noexcept;
			friend class Transcode;
			friend class Engine::Decoder::Details::Video;
			friend class Engine::Decoder::Details::Audio;
			friend class Engine::Decoder::Details::Subtitle;

		protected:
			/**
			 * @brief Prepare-once. Does not Fail if the backend is still unbound.
			 */
			void Open() noexcept override;

			/**
			 * @brief Decodes one packet and pushes frames to @ref m_out.
			 * @param item Incoming packet.
			 */
			void Work(std::shared_ptr<Item> item) noexcept override;

			/**
			 * @brief Flushes the codec after input EoF.
			 */
			void Finish() noexcept override;

		private:
			/**
			 * @brief Marks a hard error and drops the backend.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @brief Pins the opened backend. Called from demux >> decoder.
			 * @param engine Opened engine.
			 */
			void Bind(std::unique_ptr<Engine::Decoder::Engine> engine) noexcept;

			int m_index;											///< Origin track
			DecoderFlags m_flags;									///< Heuristics
			std::optional<std::string> m_language;					///< Language tag
			std::optional<std::string> m_title;						///< Title tag
			std::optional<std::string> m_implementation;			///< Pinned decoder name
			Features m_require;										///< Extra required bits
			Features m_capabilities;								///< Opened capabilities
			std::unique_ptr<Engine::Decoder::Engine> m_engine;		///< Decode backend
	};
}
