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
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <optional>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Video
 * @brief Video process filters.
 *
 * Inherit @ref Filter::Process, not @ref Filter::FFmpeg.
 * Attach with @c job.Video(in).Filter<NlMeans>(log).
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @class NlMeans
	 * @brief Spatial non-local means denoise. Process leaf.
	 *
	 * @par Algorithm
	 * One frame. Each sample is a similarity-weighted average
	 * of patches inside a search window. No temporal references,
	 * no Hold, no off-by-one.
	 *
	 * @par Defaults
	 * Empty @c research / @c patch / @c strength pick a profile
	 * from the first video frame:
	 * - 4K and above (width ≥ 3840 or height ≥ 2160):
	 *   research=2, patch=1, strength=0.8
	 * - below that (1080p class): research=3, patch=2, strength=1.0
	 *
	 * Any argument the caller sets is used as-is on every frame.
	 * There is no later clamp or “sane override”.
	 *
	 * Cost is O(width × height × research² × patch²) per plane.
	 *
	 * @par Mutation
	 * @ref Filter::FFmpeg::Save of a new
	 * @ref StormByte::Multimedia::FFmpeg::AVFrame.
	 * No avfilter `nlmeans`.
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::Process
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC NlMeans: public Filter::Process {
		public:
			/**
			 * @brief Spatial nlmeans.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param research Search-window radius. Empty → profile default.
			 * @param patch Patch radius. Empty → profile default.
			 * @param strength Filter strength h. Empty → profile default.
			 */
			NlMeans(std::shared_ptr<StormByte::Logger::Log> log,
				std::optional<unsigned> research = {},
				std::optional<unsigned> patch = {},
				std::optional<double> strength = {}) noexcept;

			NlMeans(const NlMeans& other) = delete;
			NlMeans(NlMeans&& other) noexcept = delete;
			~NlMeans() noexcept override = default;
			NlMeans& operator=(const NlMeans& other) = delete;
			NlMeans& operator=(NlMeans&& other) noexcept = delete;

			/**
			 * @brief Media this filter handles.
			 * @return Video.
			 */
			enum StormByte::Multimedia::Type Media() const noexcept override;

			void Clean() noexcept override;
			void Setup() noexcept override;
			void Process(const Pipeline::Frame& frame) noexcept override;

		private:
			/**
			 * @brief Resolves empty arguments from @p width × @p height.
			 * @param width Frame width.
			 * @param height Frame height.
			 */
			void Latch(int width, int height) noexcept;

			std::optional<unsigned> m_researchIn;	///< Caller research, or empty
			std::optional<unsigned> m_patchIn;		///< Caller patch, or empty
			std::optional<double> m_hIn;			///< Caller strength, or empty
			unsigned m_research;					///< Effective research
			unsigned m_patch;						///< Effective patch
			double m_h;								///< Effective strength
			bool m_latched;							///< Profile chosen
	};
}
