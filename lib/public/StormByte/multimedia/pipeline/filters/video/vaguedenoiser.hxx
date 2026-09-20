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
#include <StormByte/multimedia/ffmpeg/AVFilterGraph.hxx>
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Video
 * @brief Video process filters.
 *
 * Inherit @ref Filter::Process, not @ref Filter::FFmpeg.
 * Attach with @c job.Video(in).Filter<VagueDenoiser>(log).
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @class VagueDenoiser
	 * @brief Wavelet denoise via libavfilter `vaguedenoiser`.
	 *
	 * @par Algorithm
	 * 2-D wavelet shrink. Spatial only; no temporal window and no
	 * Hold. Cheaper than @ref Bm3d / @ref NlMeans / @ref Fftdnoiz
	 * on 4K. Good on fine grain after @ref Atadenoise, not as a
	 * stack with another spatial denoise.
	 *
	 * @par Defaults
	 * Empty arguments use FFmpeg's: threshold=2, nsteps=6,
	 * percent=85, method=garrote.
	 *
	 * @par Mutation
	 * @ref Filter::FFmpeg::Save of the buffersink frame.
	 *
	 * @see StormByte::Multimedia::FFmpeg::AVFilterGraph
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC VagueDenoiser: public Filter::Process {
		public:
			/**
			 * @brief Wavelet denoise (`vaguedenoiser`).
			 * @param log Shared logger. Empty pointer means no log.
			 * @param threshold Coefficient floor. Empty → 2.
			 * @param steps Wavelet steps. Empty → 6.
			 * @param percent Shrink percent. Empty → 85.
			 */
			VagueDenoiser(std::shared_ptr<StormByte::Logger::Log> log,
				std::optional<double> threshold = {},
				std::optional<unsigned> steps = {},
				std::optional<double> percent = {}) noexcept;

			VagueDenoiser(const VagueDenoiser& other) = delete;
			VagueDenoiser(VagueDenoiser&& other) noexcept = delete;
			~VagueDenoiser() noexcept override = default;
			VagueDenoiser& operator=(const VagueDenoiser& other) = delete;
			VagueDenoiser& operator=(VagueDenoiser&& other) noexcept = delete;

			/**
			 * @brief Media this filter handles.
			 * @return Video.
			 */
			enum StormByte::Multimedia::Type Media() const noexcept override;

			/**
			 * @brief Drops the cached graph.
			 */
			void Clean() noexcept override;

			/**
			 * @brief Resets the graph before the first frame.
			 */
			void Setup() noexcept override;

			/**
			 * @brief Pushes one video frame through `vaguedenoiser` and Save.
			 * @param frame Current pipeline unit. Non-video is ignored.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

		private:
			/**
			 * @brief Builds the avfilter chain.
			 * @return `vaguedenoiser=...` for @ref FFmpeg::AVFilterGraph::Ensure.
			 */
			std::string Chain() const noexcept;

			std::optional<double> m_thrIn;		///< Caller threshold, or empty
			std::optional<unsigned> m_stepsIn;	///< Caller nsteps, or empty
			std::optional<double> m_pctIn;		///< Caller percent, or empty
			std::unique_ptr<StormByte::Multimedia::FFmpeg::AVFilterGraph> m_graph;	///< Reused graph
	};
}
