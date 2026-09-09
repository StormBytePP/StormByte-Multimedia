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
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

extern "C" {
	struct AVStream;
}

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Encoder;
	class Mux;
	class Transcode;

	/**
	 * @namespace Engine
	 * @brief Private backends behind the public pipeline types.
	 *
	 * @ingroup multimedia_pipeline
	 */
	namespace Engine {
		/**
		 * @namespace Mux
		 * @brief Mux backends behind the public Mux type.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Mux {
			/**
			 * @namespace Details
			 * @brief Container mux backend.
			 *
			 * @ingroup multimedia_pipeline
			 */
			namespace Details {
				class Container;
			}
		}

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
	}

	/**
	 * @class Encoder
	 * @brief Encodes frames of one output track into packets.
	 *
	 * A @ref Step, @c final. Launches in the constructor.
	 * Codec Open is lazy on the first frame. @ref Work encodes one
	 * frame. @ref Finish flushes. @ref Index is the mux output track.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Encoder final: public Step {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Encoder for output track @p output_index and destination @p codec.
			 * @param output_index Mux track index.
			 * @param codec Registry codec. Must HasAccess(Write) at open.
			 *
			 * Launches the worker. Backend Open stays lazy.
			 */
			Encoder(int output_index, const Codec& codec) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source encoder.
			 */
			Encoder(const Encoder& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Encoder to take.
			 */
			Encoder(Encoder&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Encoder() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source encoder.
			 * @return *this.
			 */
			Encoder& operator=(const Encoder& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Encoder to take.
			 * @return *this.
			 */
			Encoder& operator=(Encoder&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @name State
			 * @{
			 */

			/**
			 * @brief true if the engine is open and not failed.
			 * @return Open and not @ref Failed.
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
			 * @brief Whether the backend finished Open().
			 * @return false before the first frame or after Fail().
			 */
			bool Opened() const noexcept;

			/**
			 * @brief Encoder channel count after Open(), audio only.
			 * @return Channels, or empty.
			 */
			std::optional<int> AudioChannels() const noexcept;

			/**
			 * @brief Encoder sample rate after Open(), audio only.
			 * @return Hz, or empty.
			 */
			std::optional<int> AudioSampleRate() const noexcept;

			/**
			 * @brief Encoder frame_size after Open(), audio only.
			 * @return Samples per packet, or empty.
			 */
			std::optional<int> AudioFrameSize() const noexcept;

			/**
			 * @brief Encoder sample format after Open(), audio only.
			 * @return AVSampleFormat as int, or empty.
			 */
			std::optional<int> AudioSampleFormat() const noexcept;

			/**
			 * @}
			 */

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
			 * @param language ISO code. Empty clears it.
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
			 * @return `StormByte-Multimedia <version>`. Never empty after construction.
			 */
			const std::string& EncoderTag() const noexcept;

			/**
			 * @}
			 */

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

			/**
			 * @}
			 */

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

			/**
			 * @}
			 */

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

			/**
			 * @}
			 */

			friend class Mux;
			friend class Transcode;
			friend class Engine::Mux::Details::Container;
			friend struct Engine::Encoder::Open::Access;
			friend class Engine::Encoder::Engine;
			friend class Engine::Encoder::Details::Video;
			friend class Engine::Encoder::Details::Audio;
			friend class Engine::Encoder::Details::Subtitle;

		protected:
			/**
			 * @brief Prepare-once. Codec Open stays lazy in @ref Work.
			 */
			void Open() noexcept override;

			/**
			 * @brief Encodes one frame and pushes packets to @ref m_out.
			 * @param item Incoming frame.
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
			 * @brief Copies codecpar / time_base onto a mux stream.
			 * @param avStream AVStream*.
			 * @return false if the encoder is not open.
			 */
			bool MuxBindStream(void* avStream) noexcept;

			int m_index;											///< Mux output track
			const Codec* m_codec;									///< Destination codec
			std::string m_encoderTag;								///< ENCODER metadata
			std::optional<std::string> m_language;					///< Language tag
			std::optional<std::string> m_title;						///< Title tag
			std::optional<std::string> m_implementation;			///< Pinned encoder name
			Features m_require;										///< Extra required bits
			Features m_capabilities;								///< Opened capabilities
			std::optional<int> m_crf;								///< CRF/CQ
			std::optional<std::int64_t> m_bitRate;					///< Target bitrate
			std::optional<std::int64_t> m_maxBitRate;				///< VBV ceiling
			std::optional<std::string> m_preset;					///< Preset
			std::optional<std::string> m_tune;						///< Tune
			std::map<std::string, std::string> m_fineTune;			///< Vendor leftovers
			std::unique_ptr<Engine::Encoder::Engine> m_engine;		///< Encode backend
	};
}
