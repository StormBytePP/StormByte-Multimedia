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
 */
namespace StormByte::Multimedia::Pipeline {
	/**
	 * @namespace Filter
	 * @brief Frame and packet steps attached to a job or a raw pipeline.
	 */
	namespace Filter {
		class FFmpeg;
	}
}

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	class Frame;
}

/**
 * @namespace StormByte::Multimedia::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Backend::FFmpeg {
	class AVDecoder;
	class AVEncoder;

	/**
	 * @class AVFrame
	 * @brief RAII owner of a libav AVFrame.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVFrame: public AVPointer<::AVFrame> {
		friend class AVDecoder;
		friend class AVEncoder;
		friend class StormByte::Multimedia::Backend::Pipeline::Frame;
		friend class StormByte::Multimedia::Pipeline::Filter::FFmpeg;
		public:
			/**
			 * @brief Allocates an empty frame (av_frame_alloc).
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
			 * @brief Move assignment. Frees *this, then takes @p other.
			 * @param other Source frame; left empty.
			 * @return *this.
			 */
			AVFrame& operator=(AVFrame&& other) noexcept = default;

			/**
			 * @brief Unreferences frame buffers (av_frame_unref).
			 */
			void Unref() noexcept;

			/**
			 * @brief Looks up side data.
			 * @param type AVFrameSideDataType value.
			 * @return Side data pointer, or nullptr.
			 */
			const AVFrameSideData* SideData(int type) const noexcept;

			/**
			 * @brief Presentation timestamp in stream ticks.
			 * @return pts, or AV_NOPTS_VALUE.
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
			 */
			void CopyPrimaryBuffer(StormByte::Buffer::DataType& out) const noexcept;

			/**
			 * @brief Writes mastering display and content light from @p hdr10.
			 * @param hdr10 High-level HDR10 bag.
			 */
			void WriteHdr10(const StormByte::Multimedia::Property::HDR10& hdr10) noexcept;

			/**
			 * @brief Copies raw SEI/side-data blobs onto the frame.
			 * @param attachments High-level side data.
			 */
			void WriteSideData(const std::vector<StormByte::Multimedia::Pipeline::SideData>& attachments) noexcept;

			/**
			 * @brief Adopts @p raw. Previous frame is freed.
			 * @param raw libav frame, or nullptr.
			 */
			void Reset(::AVFrame* raw) noexcept;

		private:
			/**
			 * @brief Deep copy via av_frame_clone.
			 * @param other Source frame.
			 */
			AVFrame(const AVFrame& other) noexcept;

			/**
			 * @brief Deep copy assignment via av_frame_clone.
			 * @param other Source frame.
			 * @return *this.
			 */
			AVFrame& operator=(const AVFrame& other) noexcept;

			/**
			 * @brief Frees the frame (av_frame_free).
			 */
			void Free() noexcept override;
	};
}
