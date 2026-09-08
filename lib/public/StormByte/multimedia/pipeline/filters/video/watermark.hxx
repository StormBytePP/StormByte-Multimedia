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

#include <StormByte/buffer/generic.hxx>
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/property/point.hxx>
#include <StormByte/multimedia/type.hxx>
#include <StormByte/multimedia/visibility.h>

#include <filesystem>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Video
 * @brief Video @ref StormByte::Multimedia::Pipeline::Filter::Process nodes.
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @enum Anchor
	 * @brief Logo placement relative to the frame.
	 */
	enum class Anchor {
		TopLeft,		///< Top left
		TopCenter,		///< Top center
		TopRight,		///< Top right
		CenterLeft,		///< Middle left
		Center,			///< Center
		CenterRight,	///< Middle right
		BottomLeft,		///< Bottom left
		BottomCenter,	///< Bottom center
		BottomRight		///< Bottom right
	};

	/**
	 * @class Watermark
	 * @brief Overlays a still image from a file. Audio and subtitles
	 *        are forwarded by
	 *        @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg::Gate.
	 *
	 * Opacity 0 is a no-op. The logo is never cropped: if it does not
	 * fit, @ref FFmpeg::Fail runs. Prefer adding this node before
	 * @ref Resize so the mark scales with the frame.
	 * Talks to libav with raw @c AVFrame* from @ref FFmpeg::Native
	 * and hands a new buffer to @ref FFmpeg::Replace.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Watermark: public Process {
		public:
			/**
			 * @brief Logo at an anchor.
			 * @param logo Path to a still image (png, jpeg, webp, bmp).
			 * @param anchor Placement.
			 * @param opacity 0–100. 0 = no-op.
			 * @param margin Pixels from the anchored edge.
			 */
			Watermark(const std::filesystem::path& logo, Anchor anchor,
				unsigned opacity = 100, int margin = 0) noexcept;

			/**
			 * @brief Logo at an absolute top-left.
			 * @param logo Path to a still image (png, jpeg, webp, bmp).
			 * @param position Top-left of the logo in frame pixels.
			 * @param opacity 0–100. 0 = no-op.
			 */
			Watermark(const std::filesystem::path& logo,
				StormByte::Multimedia::Property::Point position,
				unsigned opacity = 100) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Watermark(const Watermark&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Watermark(Watermark&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Watermark() noexcept override = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Watermark& operator=(const Watermark&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Watermark& operator=(Watermark&&) noexcept = default;

			/**
			 * @brief Media this filter handles.
			 * @return @ref StormByte::Multimedia::Type::Video.
			 */
			StormByte::Multimedia::Type Media() const noexcept override;

		protected:
			/**
			 * @brief Blends the logo onto @p frame, then @ref FFmpeg::Replace.
			 * @param frame Video unit.
			 */
			void ProcessFrame(Pipeline::Frame& frame) noexcept override;

		private:
			/**
			 * @brief Reads @ref m_path into @ref m_bytes.
			 * @return false if @ref Fail was called.
			 */
			bool LoadFile() noexcept;

			/**
			 * @brief Decodes @ref m_bytes into @ref m_rgba on first use.
			 * @return false if @ref Fail was called.
			 */
			bool DecodeLogo() noexcept;

			std::filesystem::path m_path;									///< Logo file
			std::optional<Anchor> m_anchor;									///< Relative placement
			std::optional<StormByte::Multimedia::Property::Point> m_point;	///< Absolute placement
			unsigned m_opacity;												///< 0–100
			int m_margin;													///< Anchor margin
			StormByte::Buffer::DataType m_bytes;							///< File bytes
			int m_logoWidth = 0;											///< Decoded logo width
			int m_logoHeight = 0;											///< Decoded logo height
			StormByte::Buffer::DataType m_rgba;								///< Decoded RGBA8888
			bool m_loaded = false;											///< File read attempted
			bool m_decoded = false;											///< Decode attempted
	};
}
