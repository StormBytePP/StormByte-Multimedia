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

#include <StormByte/logger/log.hxx>
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Video
 * @brief Video process filters.
 *
 * Inherit @ref Filter::Process, not @ref Filter::FFmpeg.
 * Attach with @c job.Video(in).Filter<Bwdif>(log).
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @class Bwdif
	 * @brief Bob Weaver deinterlace. Process leaf.
	 *
	 * @par Versus Yadif
	 * Same job, different interpolator (cubic spatial +
	 * temporal fallback). Do not stack @ref Yadif and
	 * @ref Bwdif on the same tube: the first already emits
	 * progressive frames, and with @c onlyInterlaced the
	 * second is a no-op. Pick one.
	 *
	 * @par Delay, not Hold
	 * Temporal bwdif needs the previous and next picture
	 * for the whole stream. @ref Hold is a one-shot window.
	 * This leaf keeps two RAII clones (@ref m_prev,
	 * @ref m_cur) and does not call Hold. The first unit
	 * waits for a neighbour; @ref Eof weaves the tail.
	 * N in, N @ref Filter::FFmpeg::Save.
	 *
	 * @par Mode
	 * One output frame per input frame (same rate).
	 *
	 * @par Mutation
	 * Builds a progressive
	 * @ref StormByte::Multimedia::FFmpeg::AVFrame from the
	 * three looks and Save. Progressive input is a no-op
	 * (no Save) when @c onlyInterlaced is set.
	 *
	 * Uses @c Data / @c Linesize / @c PlaneWidth /
	 * @c PlaneHeight / @c BitsPerComponent / @c Clone /
	 * @c AllocVideo / @c CopyProps / @c Interlaced /
	 * @c TopFieldFirst. No avfilter `bwdif`.
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::Process
	 * @see StormByte::Multimedia::Pipeline::Filter::Video::Yadif
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Bwdif: public Filter::Process {
		public:
			/**
			 * @brief BWDIF, same frame rate.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param onlyInterlaced Skip frames not marked interlaced.
			 */
			explicit Bwdif(std::shared_ptr<StormByte::Logger::Log> log,
				bool onlyInterlaced = true) noexcept;

			/**
			 * @brief Copy is not allowed. Each leaf owns delayed looks.
			 */
			Bwdif(const Bwdif& other) = delete;

			/**
			 * @brief Move is not allowed. The tube owns the mounted leaf.
			 */
			Bwdif(Bwdif&& other) noexcept = delete;

			/**
			 * @brief Drops delayed looks.
			 */
			~Bwdif() noexcept override;

			/**
			 * @brief Copy assignment is not allowed.
			 */
			Bwdif& operator=(const Bwdif& other) = delete;

			/**
			 * @brief Move assignment is not allowed.
			 */
			Bwdif& operator=(Bwdif&& other) noexcept = delete;

			/**
			 * @brief Media this filter handles.
			 * @return Video.
			 */
			enum StormByte::Multimedia::Type Media() const noexcept override;

			/**
			 * @brief Drops @ref m_prev and @ref m_cur.
			 */
			void Clean() noexcept override;

			/**
			 * @brief Calls @ref Clean. No Hold.
			 */
			void Setup() noexcept override;

			/**
			 * @brief Shifts the two-frame delay and weaves when ready.
			 * @param frame Video unit.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Weaves the delayed tail and drops it.
			 */
			void Eof() noexcept override;

			/**
			 * @brief Unused. This leaf does not Hold.
			 * @param frame Ignored.
			 */
			void LastChance(const Pipeline::Frame& frame) noexcept override;

		private:
			/**
			 * @brief Writes a progressive frame from prev/cur/next and Save.
			 * @param prev Previous picture, or empty.
			 * @param cur Current picture.
			 * @param next Next picture, or empty.
			 *
			 * Empty prev/next fall back to cubic spatial
			 * interpolation. Progressive @p cur with
			 * @ref m_onlyInterlaced set returns without Save.
			 */
			void Weave(const StormByte::Multimedia::FFmpeg::AVFrame& prev,
				const StormByte::Multimedia::FFmpeg::AVFrame& cur,
				const StormByte::Multimedia::FFmpeg::AVFrame& next) noexcept;

			bool m_onlyInterlaced;							///< Skip progressive input
			StormByte::Multimedia::FFmpeg::AVFrame m_prev;	///< Look t-1
			StormByte::Multimedia::FFmpeg::AVFrame m_cur;	///< Look t
	};
}
