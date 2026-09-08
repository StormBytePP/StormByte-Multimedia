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

#include <StormByte/buffer/typedefs.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/property/hdr10.hxx>

#include <cstdint>
#include <vector>

extern "C" {
	#include <libavutil/frame.h>
}

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * Forward-declared here so
 * @ref StormByte::Multimedia::Backend::FFmpeg::AVFrame
 * can friend the filter base and the frame engine without
 * pulling public pipeline headers into the private backend.
 */
namespace StormByte::Multimedia::Pipeline {
	/**
	 * @namespace Filter
	 * @brief Frame and packet steps attached to a job or a raw pipeline.
	 *
	 * @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg
	 * is the public friend of the private @c AVFrame copy
	 * (@ref Native / @ref Clone / @ref Replace).
	 */
	namespace Filter {
		class FFmpeg;	///< Filter base; public friend of the private copy.
	}

	/**
	 * @namespace Engine
	 * @brief Private backends. Public headers only forward-declare them.
	 */
	namespace Engine {
		/**
		 * @namespace Frame
		 * @brief Decoded-frame backend behind the public Frame type.
		 *
		 * @ref StormByte::Multimedia::Pipeline::Engine::Frame::Engine
		 * copies @c AVFrame via @c av_frame_clone when the pipeline
		 * Frame is cloned.
		 */
		namespace Frame {
			class Engine;	///< Opaque holder; friend of the private copy.
		}
	}
}

/**
 * @namespace StormByte::Multimedia::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Backend::FFmpeg {
	class AVDecoder;	///< Decode context; receives into an owned frame.
	class AVEncoder;	///< Encode context; consumes an owned frame.

	/**
	 * @class AVFrame
	 * @brief RAII owner of a libav @c AVFrame.
	 *
	 * Public API is move-only. There is no @c Clone().
	 *
	 * Copy constructor and copy assignment use @c av_frame_clone
	 * and stay private for @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg
	 * (and its nested handle) plus the decoder / encoder that already
	 * own the same pixels. A public copy would look cheap and clone
	 * every plane plus side data; Analytics that must keep a reference
	 * or a distorted frame copies
	 * @ref StormByte::Multimedia::Pipeline::Frame, not this type.
	 * Do not free @ref Get(); the destructor does.
	 *
	 * @see StormByte::Multimedia::Backend::FFmpeg::AVPointer
	 * @see StormByte::Multimedia::Pipeline::Frame
	 * @see StormByte::Multimedia::Pipeline::Filter::FFmpeg
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVFrame: public AVPointer<::AVFrame> {
		friend class AVDecoder;
		friend class AVEncoder;
		friend class StormByte::Multimedia::Pipeline::Filter::FFmpeg;
		friend class StormByte::Multimedia::Pipeline::Engine::Frame::Engine;
		public:
			/**
			 * @brief Allocates an empty frame (@c av_frame_alloc).
			 */
			AVFrame() noexcept;

			/**
			 * @brief Move constructor. Transfers the libav pointer.
			 * @param other Source frame; left empty.
			 */
			AVFrame(AVFrame&& other) noexcept = default;

			/**
			 * @brief Destructor. Unreferences buffers and frees the struct.
			 */
			~AVFrame() noexcept override;

			/**
			 * @brief Move assignment. Frees @c *this, then takes @p other.
			 * @param other Source frame; left empty.
			 * @return *this.
			 */
			AVFrame& operator=(AVFrame&& other) noexcept = default;

			/**
			 * @brief Unreferences frame buffers (@c av_frame_unref).
			 *
			 * The @c AVFrame struct stays allocated; planes and side data
			 * are released. Use before @c avcodec_receive_frame reuse.
			 */
			void Unref() noexcept;

			/**
			 * @brief Looks up side data.
			 * @param type @c AVFrameSideDataType value.
			 * @return Side data pointer, or nullptr.
			 */
			const AVFrameSideData* SideData(int type) const noexcept;

			/**
			 * @brief Presentation timestamp in stream ticks.
			 * @return @c pts, or @c AV_NOPTS_VALUE.
			 */
			std::int64_t Pts() const noexcept;

			/**
			 * @brief Duration in stream ticks.
			 * @return Duration, or 0.
			 */
			std::int64_t DurationTicks() const noexcept;

			/**
			 * @brief Packs planar video/audio into @p out without linesize padding.
			 * @param out Destination (cleared first).
			 *
			 * Video uses @c av_image_copy_to_buffer; audio uses
			 * @c av_samples_copy. No-op when the frame has no planes.
			 */
			void CopyPrimaryBuffer(StormByte::Buffer::DataType& out) const noexcept;

			/**
			 * @brief Writes mastering display and content light from @p hdr10.
			 * @param hdr10 High-level HDR10 bag.
			 *
			 * Replaces existing MDM/CLL side data. No-op if @p hdr10 is empty.
			 */
			void WriteHdr10(const StormByte::Multimedia::Property::HDR10& hdr10) noexcept;

			/**
			 * @brief Copies raw SEI/side-data blobs onto the frame.
			 * @param attachments High-level side data.
			 *
			 * MDM/CLL are skipped (@ref WriteHdr10 owns those). HDR10+ is written
			 * as @c AV_FRAME_DATA_DYNAMIC_HDR_PLUS via the libav helper.
			 */
			void WriteSideData(const std::vector<StormByte::Multimedia::Pipeline::SideData>& attachments) noexcept;

		private:
			/**
			 * @brief Deep copy via @c av_frame_clone.
			 * @param other Source frame.
			 *
			 * Private on purpose: a public copy of this RAII type would
			 * silently duplicate every plane. Only
			 * @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg
			 * (nested handle / @c Native path) and the codec friends
			 * may clone. Callers that need a second decoded unit copy
			 * @ref StormByte::Multimedia::Pipeline::Frame.
			 */
			AVFrame(const AVFrame& other) noexcept;

			/**
			 * @brief Deep copy assignment via @c av_frame_clone.
			 * @param other Source frame.
			 * @return *this.
			 *
			 * Same restriction as the copy constructor: private so the
			 * expensive clone cannot be invoked from pipeline user code.
			 */
			AVFrame& operator=(const AVFrame& other) noexcept;

			/**
			 * @brief Frees the frame (@c av_frame_free).
			 */
			void Free() noexcept override;
	};
}
