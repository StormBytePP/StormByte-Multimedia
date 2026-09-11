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
#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/property/audio.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/property/video.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	/**
     * @class Frame
     * @brief One decoded access unit.
     *
     * In libav that is an @c AVFrame: one picture or one block of
     * samples after decode. In Multimedia it is what process and
     * analytics filters, and @ref Encoder, see — pixels or samples,
     * timestamps and the stream tags the decoder copied across.
     *
     * @see Item
     * @see Packet
     *
     * @ingroup multimedia_pipeline
     */
	class STORMBYTE_MULTIMEDIA_PUBLIC Frame: public Item {
		public:
			/**
			 * @name Construction
			 * @{
			 */

			/**
			 * @brief Empty frame.
			 *
			 * @ref Item::Type is @ref StormByte::Multimedia::Type::Unknown.
			 * @ref Item::Track is -1. @ref Item::Kind is @ref Kind::Frame.
			 */
			Frame() noexcept;

			/**
			 * @brief Builds a frame without a backend buffer.
			 * @param track Origin container stream index.
			 * @param type Media of this unit (Video, Audio or Subtitle).
			 * @param producer Step that created this unit.
			 * @param payload Owned sample / plane bytes.
			 * @param pts Presentation timestamp, if known.
			 * @param duration Frame duration, if known.
			 * @param video Video properties, if this is a video frame.
			 * @param attachments Raw side-data blobs.
			 * @param audio Audio properties, if this is an audio frame.
			 */
			Frame(int track, enum StormByte::Multimedia::Type type, enum Producer producer,
				StormByte::Buffer::FIFO payload,
				std::optional<Property::Duration> pts = std::nullopt,
				std::optional<Property::Duration> duration = std::nullopt,
				std::optional<Property::Video> video = std::nullopt,
				std::vector<class SideData> attachments = {},
				std::optional<Property::Audio> audio = std::nullopt) noexcept;

			/**
			 * @brief Move constructor.
			 * @param other Frame to take.
			 *
			 * Container-safe: @p other becomes the empty sentinel
			 * (@ref Item::Type Unknown, track -1, no backend). @c ~Frame
			 * on a moved-from object is a no-op. Copy stays private.
			 */
			Frame(Frame&& other) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Frame() noexcept override;

			/**
			 * @brief Move assignment.
			 * @param other Frame to take.
			 * @return *this.
			 *
			 * Same as the move constructor: @p other is left empty.
			 */
			Frame& operator=(Frame&& other) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Timing and identity
			 * @{
			 */

			/**
			 * @brief Presentation timestamp on the stream clock.
			 * @return Pts, or empty.
			 */
			const std::optional<Property::Duration>& Pts() const noexcept;

			/**
			 * @brief Frame duration on the stream clock.
			 * @return Duration, or empty.
			 */
			const std::optional<Property::Duration>& Duration() const noexcept;

			/**
			 * @brief Stream language tag copied from File metadata.
			 * @return Language, or empty if the stream had none.
			 */
			const std::optional<std::string>& Language() const noexcept;

			/**
			 * @brief Sets the stream language tag.
			 * @param language ISO code from File metadata (`spa`, `eng`, `es`, …).
			 *        Empty clears it.
			 */
			void Language(std::string language) noexcept;

			/**
			 * @brief Stream title tag copied from File metadata.
			 * @return Title, or empty if the stream had none.
			 */
			const std::optional<std::string>& Title() const noexcept;

			/**
			 * @brief Sets the stream title tag.
			 * @param title Title from File metadata. Empty clears it.
			 */
			void Title(std::string title) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Properties
			 * @{
			 */

			/**
			 * @brief Video properties (includes HDR10 when set).
			 * @return Video, or empty.
			 */
			const std::optional<Property::Video>& Video() const noexcept;

			/**
			 * @brief Audio properties (layout, rate, channels).
			 * @return Audio, or empty.
			 */
			const std::optional<Property::Audio>& Audio() const noexcept;

			/**
			 * @brief Raw side data captured at receive. Read-only.
			 * @return Blobs. MDM/CLL also appear in @ref Video() HDR10
			 *         when the decoder could map them.
			 */
			const std::vector<class SideData>& Attachments() const noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Payload
			 * @{
			 */

			/**
			 * @brief Payload. Materialises planes on first call if a backend frame is held.
			 * @return FIFO.
			 *
			 * After materialisation the backend stays alive; a later
			 * filter @c Save replaces it and this FIFO is cleared.
			 */
			StormByte::Buffer::FIFO& Payload() noexcept;

			/**
			 * @brief Payload already materialised, or empty.
			 * @return FIFO. Does not pull planes out of the backend.
			 */
			const StormByte::Buffer::FIFO& Payload() const noexcept;

			/**
			 * @}
			 */

		friend class Decoder;
		friend Decoder& operator>>(Decoder& decoder, Frame& frame) noexcept;
		friend class Encoder;
		friend Frame& operator>>(Frame& frame, Encoder& encoder) noexcept;
		friend class Filter::FFmpeg;
		friend struct Engine::Encoder::Open::Access;
		friend class Engine::Encoder::Details::Video;
		friend class Engine::Encoder::Details::Audio;
		friend class Engine::Encoder::Details::Subtitle;
		friend class Engine::Decoder::Details::Video;
		friend class Engine::Decoder::Details::Audio;
		friend class Engine::Decoder::Details::Subtitle;
		friend class Engine::Frame::Engine;

		private:
			/**
			 * @name Construction
			 * @{
			 */

			/**
			 * @brief Deep copy (metadata, FIFO and cloned @c AVFrame).
			 * @param other Source frame.
			 *
			 * Private on purpose: a public copy of a decoded unit
			 * would silently duplicate every plane. Only
			 * @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg
			 * and the codec friends may clone.
			 * There is no public @c Clone().
			 */
			Frame(const Frame& other) noexcept;

			/**
			 * @brief Deep copy assignment (metadata, FIFO and cloned @c AVFrame).
			 * @param other Source frame.
			 * @return *this.
			 *
			 * Same restriction as the copy constructor: private so the
			 * expensive clone cannot be invoked from pipeline user code.
			 */
			Frame& operator=(const Frame& other) noexcept;

			/**
			 * @}
			 */

			StormByte::Buffer::FIFO m_payload;							///< Sample / subtitle bytes
			std::optional<Property::Duration> m_pts;					///< Presentation timestamp
			std::optional<Property::Duration> m_duration;				///< Frame duration
			std::optional<Property::Video> m_video;						///< Video properties
			std::optional<Property::Audio> m_audio;						///< Audio properties
			std::optional<std::string> m_language;						///< Stream language tag
			std::optional<std::string> m_title;							///< Stream title tag
			std::vector<class SideData> m_attachments;					///< Raw side data
			std::unique_ptr<Engine::Frame::Engine> m_engine;			///< Backend holder

			/**
			 * @brief Adopts a backend frame for lazy @ref Payload().
			 * @param engine Backend holder.
			 */
			void Bind(std::unique_ptr<Engine::Frame::Engine> engine) noexcept;

			/**
			 * @brief Turns this unit into the empty sentinel.
			 *
			 * Used by move construction and move assignment so a
			 * relocated @c Frame in a container is always valid.
			 */
			void BecomeEmpty() noexcept;
	};
}
