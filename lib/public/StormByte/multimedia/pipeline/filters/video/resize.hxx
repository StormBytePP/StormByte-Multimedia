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

#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/property/resolution.hxx>
#include <StormByte/multimedia/type.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter
 * @brief Frame and packet steps attached to a job or a raw pipeline.
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @class Resize
	 * @brief Scales a video frame. Audio and subtitles are forwarded by
	 *        @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg::Gate.
	 *
	 * Width or height 0 keeps the source aspect ratio. Both 0 fails
	 * on the first video frame. Talks to libav with raw @c AVFrame*
	 * from @ref FFmpeg::Native and hands the result to
	 * @ref FFmpeg::Replace. Does not use Multimedia RAII wrappers.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Resize: public Process {
		public:
			/**
			 * @brief Exact destination size.
			 * @param resolution Target resolution.
			 */
			explicit Resize(const StormByte::Multimedia::Property::Resolution& resolution) noexcept;

			/**
			 * @brief Destination size. 0 on one axis keeps aspect ratio.
			 * @param width Target width, or 0.
			 * @param height Target height, or 0.
			 */
			Resize(std::uint32_t width, std::uint32_t height) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Resize(const Resize&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Resize(Resize&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Resize() noexcept override = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Resize& operator=(const Resize&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Resize& operator=(Resize&&) noexcept = default;

			/**
			 * @brief Media this filter handles.
			 * @return @ref StormByte::Multimedia::Type::Video.
			 */
			StormByte::Multimedia::Type Media() const noexcept override;

		protected:
			/**
			 * @brief Scales @p frame in place.
			 * @param frame Video unit.
			 */
			void ProcessFrame(Pipeline::Frame& frame) noexcept override;

		private:
			std::uint32_t m_width;		///< Requested width, 0 = auto
			std::uint32_t m_height;		///< Requested height, 0 = auto
	};
}
