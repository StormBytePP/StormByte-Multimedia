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
#include <StormByte/multimedia/attachment.hxx>
#include <StormByte/multimedia/pipeline/filters/frame.hxx>
#include <StormByte/multimedia/property/point.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter
 * @brief Packet and frame steps. Bundled or user-supplied.
 */
namespace StormByte::Multimedia::Pipeline::Filter {
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
		CenterRight,		///< Middle right
		BottomLeft,		///< Bottom left
		BottomCenter,		///< Bottom center
		BottomRight		///< Bottom right
	};

	/**
	 * @class Watermark
	 * @brief Overlays a still image. Audio and subtitles pass through.
	 *
	 * Opacity 0 is a real no-op (no decode, no blend). The logo is never
	 * cropped: if it does not fit, Push fails. Prefer Add(Watermark)
	 * before Add(Resize) so the mark scales with the frame.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Watermark: public Step {
		public:
			/**
			 * @brief Logo at an anchor.
			 * @param logo Still image attachment. Payload is peeked, the object is not copied.
			 * @param anchor Placement.
			 * @param opacity 0–100. 0 = no-op.
			 * @param margin Pixels from the anchored edge.
			 */
			Watermark(const StormByte::Multimedia::Attachment& logo, Anchor anchor,
				unsigned opacity = 100, int margin = 0) noexcept;

			/**
			 * @brief Logo at an absolute top-left.
			 * @param logo Still image attachment. Payload is peeked, the object is not copied.
			 * @param position Top-left of the logo in frame pixels.
			 * @param opacity 0–100. 0 = no-op.
			 */
			Watermark(const StormByte::Multimedia::Attachment& logo,
				StormByte::Multimedia::Property::Point position,
				unsigned opacity = 100) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Watermark(const Watermark&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Watermark(Watermark&&) noexcept;

			/**
			 * @brief Destructor.
			 */
			~Watermark() noexcept override;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Watermark& operator=(const Watermark&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Watermark& operator=(Watermark&&) noexcept;

			/**
			 * @brief Overlays the logo on a video frame.
			 * @param frame Incoming frame.
			 * @return Frame with logo, or empty on failure.
			 */
			std::optional<Pipeline::Frame> Push(Pipeline::Frame&& frame) noexcept override;

		private:
			class Impl;

			StormByte::Buffer::DataType m_bytes;					///< Logo file bytes
			std::optional<std::string> m_mime;					///< Logo mime
			std::optional<std::string> m_name;					///< Logo filename
			std::optional<Anchor> m_anchor;						///< Relative placement
			std::optional<StormByte::Multimedia::Property::Point> m_point;		///< Absolute placement
			unsigned m_opacity;							///< 0–100
			int m_margin;								///< Anchor margin
			std::unique_ptr<Impl> m_impl;						///< Decoded logo
	};
}
