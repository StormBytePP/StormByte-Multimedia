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

#include <cstdint>
#include <filesystem>
#include <optional>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Video
 * @brief Video process filters.
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @enum Anchor
	 * @brief Logo placement relative to the active picture.
	 *
	 * Top / Bottom / Left / Right are edges of the measured
	 * letterbox / pillarbox rectangle, not of the full @c AVFrame.
	 */
	enum class STORMBYTE_MULTIMEDIA_PUBLIC Anchor {
		TopLeft,		///< Top left of the active picture
		TopCenter,		///< Top center of the active picture
		TopRight,		///< Top right of the active picture
		CenterLeft,		///< Middle left of the active picture
		Center,			///< Center of the active picture
		CenterRight,	///< Middle right of the active picture
		BottomLeft,		///< Bottom left of the active picture
		BottomCenter,	///< Bottom center of the active picture
		BottomRight		///< Bottom right of the active picture
	};

	/**
	 * @class Watermark
	 * @brief Overlays a still image from a file on decoded video.
	 *
	 * Opacity 0 is a no-op. The logo is never cropped: if it does not
	 * fit the active picture, @ref FFmpeg::Fail runs.
	 *
	 * Works on every software pixel format the decoder emits. Bars
	 * are measured on a luma plane produced by libswscale, not by
	 * reading @c data[0] of the source. Overlay converts to RGBA and
	 * back to the source format.
	 *
	 * With an @ref Anchor the first run opens @ref FFmpeg::Hold with a
	 * plugin-chosen maximum. Near-black slates do not update the
	 * rectangle. Bar widths accumulate (max). @ref FFmpeg::Release may
	 * run before the ceiling when the rectangle stops changing. An
	 * absolute @ref StormByte::Multimedia::Property::Point does not Hold.
	 *
	 * Talks to libav with raw @c AVFrame* from @ref FFmpeg::AVFrame
	 * and @ref FFmpeg::Save.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Watermark: public Process {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Logo at an anchor on the active picture.
			 * @param logo Path to a still image (png, jpeg, webp, bmp).
			 * @param anchor Placement.
			 * @param opacity 0–100. 0 = no-op.
			 * @param margin Pixels from the anchored active edge.
			 */
			Watermark(const std::filesystem::path& logo, Anchor anchor,
				unsigned opacity = 100, int margin = 0) noexcept;

			/**
			 * @brief Logo at an absolute top-left. No Hold.
			 * @param logo Path to a still image (png, jpeg, webp, bmp).
			 * @param position Top-left of the logo in frame pixels.
			 * @param opacity 0–100. 0 = no-op.
			 */
			Watermark(const std::filesystem::path& logo,
				StormByte::Multimedia::Property::Point position,
				unsigned opacity = 100) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source filter.
			 */
			Watermark(const Watermark& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Filter to take.
			 */
			Watermark(Watermark&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Watermark() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source filter.
			 * @return *this.
			 */
			Watermark& operator=(const Watermark& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Filter to take.
			 * @return *this.
			 */
			Watermark& operator=(Watermark&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @name Identity
			 * @{
			 */

			/**
			 * @brief Media this filter handles.
			 * @return Video.
			 */
			enum StormByte::Multimedia::Type Media() const noexcept override;

			/**
			 * @}
			 */

		protected:
			/**
			 * @name Run
			 * @{
			 */

			/**
			 * @brief Drops decoded logo and bar state from a previous run.
			 */
			void Clean() noexcept override;

			/**
			 * @brief Reads the logo file for the coming run.
			 */
			void Setup() noexcept override;

			/**
			 * @brief Holds to measure bars when needed, blends, @ref FFmpeg::Save.
			 * @param frame Video unit.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @}
			 */

		private:
			static constexpr std::uint8_t ProbeMax = 200;	///< Plugin Hold ceiling

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

			/**
			 * @brief Builds an 8-bit luma view of @p src for bar sampling.
			 * @param src Live libav frame.
			 * @return Luma frame, or nullptr on Fail.
			 */
			::AVFrame* Luma(::AVFrame* src) noexcept;

			/**
			 * @brief Samples letterbox / pillarbox on the luma view of @p src.
			 * @param src Live libav frame.
			 * @return true if this frame updated or confirmed the rectangle.
			 */
			bool ProbeBars(::AVFrame* src) noexcept;

			/**
			 * @brief Paints the logo and @ref FFmpeg::Save.
			 */
			void Paint() noexcept;

			/**
			 * @brief Frees cached scale contexts and luma buffer.
			 */
			void DropScale() noexcept;

			std::filesystem::path m_path;									///< Logo file
			std::optional<Anchor> m_anchor;									///< Relative placement
			std::optional<StormByte::Multimedia::Property::Point> m_point;	///< Absolute placement
			unsigned m_opacity;												///< 0–100
			int m_margin;													///< Anchor margin
			StormByte::Buffer::DataType m_bytes;							///< File bytes
			int m_logoWidth;												///< Decoded logo width
			int m_logoHeight;												///< Decoded logo height
			StormByte::Buffer::DataType m_rgba;								///< Decoded RGBA8888
			bool m_loaded;													///< File read attempted
			bool m_decoded;													///< Decode attempted
			bool m_released;												///< Hold finished for this run
			int m_barTop;													///< Letterbox top
			int m_barBottom;												///< Letterbox bottom
			int m_barLeft;													///< Pillarbox left
			int m_barRight;													///< Pillarbox right
			int m_stable;													///< Consecutive unchanged probes
			int m_lumaW;													///< Cached luma width
			int m_lumaH;													///< Cached luma height
			int m_lumaFmt;													///< Cached source pixel format
			void* m_swsLuma;												///< Cached src → gray SwsContext
			::AVFrame* m_luma;												///< Cached GRAY8 view
	};
}
