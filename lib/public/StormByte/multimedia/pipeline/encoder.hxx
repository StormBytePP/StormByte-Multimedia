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

#include <StormByte/buffer/fifo.hxx>
#include <StormByte/logger/log.hxx>
#include <StormByte/multimedia/codec.hxx>
#include <StormByte/multimedia/features.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	class Encoder;	///< Encode backend behind @ref StormByte::Multimedia::Pipeline::Encoder.
	class Packet;	///< Packet holder behind the public Packet.
}

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Encoder;
	class Frame;
	class Muxer;

	/**
	 * @class Encoder
	 * @brief Encodes frames of one output track into packets.
	 *
	 * Notice: destination codec when the backend first opens.
	 * LowLevel unit lines use @ref Step::Sparse /
	 * @ref Step::MaybeThrottle keyed by mux output index
	 * (@ref Index). One incoming frame counts once; packets
	 * produced in that Work share the sample. Pts/Dts on Wrap
	 * are the flattened encoder clock (the line that catches a
	 * bad DTS flatten). Step::Pump times each Work.
	 *
	 * @ref Label is `Encoder(libx265)` when an implementation is
	 * pinned or selected, otherwise `Encoder(<registry name>)`.
	 *
	 * Encode-look is @ref Step::Look. This class overrides it
	 * privately. It is not a Tee of @ref m_out and it is not a
	 * public Encoder method.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Encoder final: public Step {
		friend class Backend::Pipeline::Encoder;
		friend class Muxer;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Encoder for output track @p output_index and destination @p codec.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param output_index Mux destination order key.
			 * @param codec Registry codec. Must HasAccess(Write) at open.
			 */
			Encoder(std::shared_ptr<StormByte::Logger::Log> log,
				int output_index, const Codec& codec) noexcept;

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
			 * @brief true if the backend is open and not failed.
			 * @return Open and not Failed.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @brief Mux destination order key.
			 * @return Index set at construction.
			 */
			inline int Index() const noexcept {
				return m_index;
			}

			/**
			 * @brief Ceiling of the encoder input hopper.
			 * @return Max queued frames. Never 0.
			 */
			std::size_t InputCeiling() const noexcept override {
				return Ceiling;
			}

			/**
			 * @brief Destination codec.
			 * @return Registry codec bound at construction.
			 */
			inline const Codec& Destination() const noexcept {
				return *m_codec;
			}

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
			 * @name Encoder tag
			 * @{
			 */

			/**
			 * @brief Writing-application tag stamped on every encoded stream.
			 * @return StormByte-Multimedia version. Never empty after construction.
			 */
			inline const std::string& EncoderTag() const noexcept {
				return m_encoderTag;
			}

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
			inline const std::optional<std::string>& Implementation() const noexcept {
				return m_implementation;
			}

			/**
			 * @brief Pins an FFmpeg encoder name. Empty clears the pin.
			 * @param name avcodec_find_encoder_by_name key.
			 */
			void Implementation(std::string name) noexcept;

			/**
			 * @brief Extra features the caller demands besides Frame HDR.
			 * @return Mask.
			 */
			inline const Features& Require() const noexcept {
				return m_require;
			}

			/**
			 * @brief Replaces the extra feature mask (before open).
			 * @param features Required bits.
			 */
			inline void Require(Features features) noexcept {
				m_require = features;
			}

			/**
			 * @brief Features of the row selected at open. Empty if fallback.
			 * @return Mask.
			 */
			inline const Features& Capabilities() const noexcept {
				return m_capabilities;
			}

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
			inline void CRF(int value) noexcept {
				m_crf = value;
			}

			/**
			 * @brief CRF/CQ, if set.
			 * @return Value, or empty.
			 */
			inline const std::optional<int>& CRF() const noexcept {
				return m_crf;
			}

			/**
			 * @brief Target bitrate in bits per second. Incompatible with CRF.
			 * @param bits_per_second Bitrate.
			 */
			inline void BitRate(std::int64_t bits_per_second) noexcept {
				m_bitRate = bits_per_second;
			}

			/**
			 * @brief Target bitrate, if set.
			 * @return Bits per second, or empty.
			 */
			inline const std::optional<std::int64_t>& BitRate() const noexcept {
				return m_bitRate;
			}

			/**
			 * @brief VBV ceiling in bits per second.
			 * @param bits_per_second Max bitrate.
			 */
			inline void MaxBitRate(std::int64_t bits_per_second) noexcept {
				m_maxBitRate = bits_per_second;
			}

			/**
			 * @brief VBV ceiling, if set.
			 * @return Bits per second, or empty.
			 */
			inline const std::optional<std::int64_t>& MaxBitRate() const noexcept {
				return m_maxBitRate;
			}

			/**
			 * @brief Encoder preset.
			 * @param name Preset name.
			 */
			void Preset(std::string name) noexcept;

			/**
			 * @brief Preset, if set.
			 * @return Name, or empty.
			 */
			inline const std::optional<std::string>& Preset() const noexcept {
				return m_preset;
			}

			/**
			 * @brief Content tune.
			 * @param name Tune name.
			 */
			void Tune(std::string name) noexcept;

			/**
			 * @brief Content tune, if set.
			 * @return Name, or empty.
			 */
			inline const std::optional<std::string>& Tune() const noexcept {
				return m_tune;
			}

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
			inline const std::map<std::string, std::string>& FineTune() const noexcept {
				return m_fineTune;
			}

			/**
			 * @brief Replaces the vendor dict (before open).
			 * @param options Key/value pairs.
			 */
			inline void FineTune(std::map<std::string, std::string> options) noexcept {
				m_fineTune = std::move(options);
			}

			/**
			 * @}
			 */

		private:
			/**
			 * @name Logging
			 * @{
			 */

			using Step::Log;

			/**
			 * @brief Token after `STMM ` for this encoder.
			 * @return `Encoder(<implementation>)` when pinned or selected,
			 *         otherwise `Encoder(<registry codec name>)`.
			 */
			std::string Label() const noexcept override;

			/**
			 * @}
			 */

			/**
			 * @brief Prepare-once. Codec Open stays lazy in Work.
			 */
			void Open() noexcept override;

			/**
			 * @brief Encodes one frame and pushes packets to m_out.
			 * @param item Incoming frame.
			 */
			void Work(std::shared_ptr<Item> item) noexcept override;

			/**
			 * @brief Flushes the codec after input EoF.
			 */
			void Finish() noexcept override;

			/**
			 * @brief Encode-look side channel. Deep-copies each encoded packet into @p sink.
			 * @param sink Look decoder @c m_in.
			 *
			 * Overrides the Step no-op. Not a Tee of @ref m_out.
			 * @ref Route::TapEncode calls the Step hook.
			 */
			void Look(StormByte::Multimedia::Buffer::Sink& sink) noexcept override;

			/**
			 * @brief Builds an encoded packet. Called from the encode backend.
			 * @param type Destination codec type.
			 * @param index Mux destination order key.
			 * @param payload Compressed bytes.
			 * @param pts Presentation time.
			 * @param dts Decode time.
			 * @param duration Packet duration.
			 * @param keyFrame Whether this is a keyframe.
			 * @param attachments Mapped packet side data.
			 * @param backend Holder with the libav packet and codecpar.
			 * @return Packet with Producer::Encoder. Empty if no lineage is latched.
			 *
			 * Copies @ref Frame::Serial and @ref Frame::Part of the last
			 * accepted frame. This is pipe lineage, not a packet count.
			 * Logs flattened pts/dts at LowLevel when the current Work
			 * is inside the Sparse window of @ref Index.
			 * Binds @p backend so Analytics can open a decoder from
			 * codecpar (extradata included).
			 */
			std::shared_ptr<Packet> Wrap(
				enum StormByte::Multimedia::Type type, int index,
				StormByte::Buffer::FIFO payload,
				std::optional<Property::Duration> pts,
				std::optional<Property::Duration> dts,
				std::optional<Property::Duration> duration,
				bool keyFrame,
				std::vector<SideData> attachments,
				std::unique_ptr<Backend::Pipeline::Packet> backend) noexcept;

			/**
			 * @brief Copies codecpar / time_base onto a mux stream.
			 * @param avStream Opaque AVStream*.
			 * @return false if the encoder is not open.
			 */
			bool MuxBindStream(void* avStream) noexcept;

			/**
			 * @brief libav frame bound to @p frame, if any.
			 * @param frame Public unit.
			 * @return Backend handle, or nullptr.
			 */
			void* FrameHandle(Frame& frame) noexcept;

			/**
			 * @brief libav frame bound to @p frame, if any.
			 * @param frame Public unit.
			 * @return Backend handle, or nullptr.
			 */
			const void* FrameHandle(const Frame& frame) noexcept;

			/**
			 * @brief Deep-copies @p packet to the look sink, then pushes the original to @ref m_out.
			 * @param packet Encoded unit. Empty pointers are ignored.
			 */
			void Emit(std::shared_ptr<Packet> packet) noexcept;

			static constexpr std::size_t Ceiling = 64;							///< Input hopper ceiling
			int m_index;														///< Mux destination order key
			const Codec* m_codec;												///< Destination codec
			std::string m_encoderTag;											///< ENCODER metadata
			std::optional<std::string> m_implementation;						///< Pinned encoder name
			Features m_require;													///< Extra required bits
			Features m_capabilities;											///< Opened capabilities
			std::optional<int> m_crf;											///< CRF/CQ
			std::optional<std::int64_t> m_bitRate;								///< Target bitrate
			std::optional<std::int64_t> m_maxBitRate;							///< VBV ceiling
			std::optional<std::string> m_preset;								///< Preset
			std::optional<std::string> m_tune;									///< Tune
			std::map<std::string, std::string> m_fineTune;						///< Vendor leftovers
			std::unique_ptr<Backend::Pipeline::Encoder> m_backend;				///< Encode backend
			std::optional<std::uint64_t> m_serial;								///< Lineage of the last accepted frame
			std::uint64_t m_part;												///< Part of the last accepted frame
			bool m_talk;														///< Sparse window of the current Work
			std::unique_ptr<StormByte::Multimedia::Buffer::Sink> m_lookOut;		///< Encode-look producer; not m_out
	};
}
