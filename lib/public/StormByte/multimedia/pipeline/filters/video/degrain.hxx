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
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Video
 * @brief Video process filters.
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @class Degrain
	 * @brief Two-pass, per-stretch film-grain reduction. ProcessTwoPasses leaf.
	 *
	 * Attach with @c job.Video(in).Filter<Degrain>(log).
	 *
	 * @par What it is for
	 * A scan or grade where grain **comes and goes**: a dark
	 * interior is dirty, a bright exterior is almost clean.
	 * Degrain reads the clip once, splits it into stretches at
	 * luma jumps (scene cuts), votes a sigma **per stretch**,
	 * then reads it again and runs libavfilter `fftdnoiz` only
	 * where that vote is stable. A quiet stretch can stay
	 * untouched while a heavy one is hit harder.
	 *
	 * @par Versus @ref Fftdnoiz
	 * @ref Fftdnoiz is the one-pass Process leaf: **you** pick
	 * one sigma (and prev/next) for the whole tube. Degrain is
	 * not a copy of that class, not a subclass, and not a thin
	 * wrapper that forwards to `Filter<Fftdnoiz>`. It is a
	 * different contract (`ProcessTwoPasses`): this leaf
	 * **computes** the sigma list. The second pass happens to
	 * use the same avfilter string (`fftdnoiz=sigma=:prev=1:next=1:…`)
	 * because that is the engine, not because the two leaves
	 * are the same filter.
	 *
	 * Use @ref Fftdnoiz when the sigma is already known and
	 * uniform. Use Degrain when the grain level has to be
	 * discovered and may change mid-clip.
	 *
	 * @par What it is not
	 * Not @ref StormByte::Multimedia::Pipeline::Filter::Audio::Denoise
	 * (room tone). Not Atadenoise / Bm3d / NlMeans.
	 *
	 * @par Measure
	 * Each frame is ScaleTo gray8. Flat 3×3 patches (low local
	 * contrast) contribute a residual RMS, mapped to an
	 * `fftdnoiz` sigma. A jump in mean luma closes the current
	 * stretch and opens the next. Stretches with too few flats
	 * or an unstable cluster stay marked skip.
	 *
	 * @par Process
	 * Lookup by PTS. Skip stretch → no Save. Otherwise
	 * `fftdnoiz` with that stretch’s sigma. Ensure rebuilds
	 * the graph when sigma changes. Hardware Fail.
	 *
	 * @par Cost
	 * Expensive because `fftdnoiz` is a 3-D FFT, not because
	 * there are two passes. The extra pass is a cheap luma
	 * probe.
	 *
	 * @par Do not stack
	 * Degrain **or** @ref Fftdnoiz / Bm3d / NlMeans /
	 * Atadenoise, not both. Cas may follow.
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::ProcessTwoPasses
	 * @see StormByte::Multimedia::Pipeline::Filter::Video::Fftdnoiz
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Degrain: public Filter::ProcessTwoPasses {
		public:
			/**
			 * @brief Automatic per-stretch film-grain `fftdnoiz`.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param sigmaCap Upper clamp for every voted sigma. Empty → 8.
			 *        Clamped to 0–100 (FFmpeg `fftdnoiz` range).
			 */
			Degrain(std::shared_ptr<StormByte::Logger::Log> log,
				std::optional<double> sigmaCap = {}) noexcept;

			/**
			 * @brief Copy is not allowed. The graph is bound to one tube.
			 */
			Degrain(const Degrain& other) = delete;

			/**
			 * @brief Move is not allowed. The tube owns the mounted leaf.
			 */
			Degrain(Degrain&& other) noexcept = delete;

			/**
			 * @brief Drops the luma probe and the cached graph.
			 */
			~Degrain() noexcept override = default;

			/**
			 * @brief Copy assignment is not allowed.
			 */
			Degrain& operator=(const Degrain& other) = delete;

			/**
			 * @brief Move assignment is not allowed.
			 */
			Degrain& operator=(Degrain&& other) noexcept = delete;

			/**
			 * @brief Media this filter handles.
			 * @return Video.
			 */
			enum StormByte::Multimedia::Type Media() const noexcept override;

			/**
			 * @brief Drops stretches, vote, luma probe and graph.
			 */
			void Clean() noexcept override;

			/**
			 * @brief Resets the graph before a pass. Keeps the vote.
			 */
			void Setup() noexcept override;

			/**
			 * @brief First pass: split stretches and collect sigmas. No Save.
			 * @param frame Current pipeline unit. Non-video is ignored.
			 */
			void Measure(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Second pass: `fftdnoiz` for the stretch under @p frame, or no-op.
			 * @param frame Current pipeline unit. Non-video is ignored.
			 *
			 * @par No-op
			 * Stretch marked skip, or no stretch for this PTS:
			 * returns without Save so the tube keeps the picture.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Closes the last stretch vote (measure) or flushes the graph (process).
			 */
			void Eof() noexcept override;

			/**
			 * @brief How many stretches ran, skipped, and the last sigma.
			 * @return Ok after measure (including all-skip), else Failed.
			 */
			class Filter::Report Report() const noexcept override;

		private:
			/**
			 * @brief One scene stretch: PTS span + sigma vote.
			 */
			struct Segment {
				int64_t firstPts = 0;	///< First PTS in this stretch
				int64_t lastPts = 0;	///< Last PTS seen in Measure
				std::vector<double> cand;	///< Flat-residual sigmas
				double sigma = 0.0;	///< Voted sigma after VoteSeg
				int matches = 0;	///< Neighbours of the winner
				bool skip = true;	///< Process is a no-op here
			};

			/**
			 * @brief Scores one frame’s flat residual into the current stretch.
			 * @param src Current software video frame.
			 */
			void Emit(const StormByte::Multimedia::FFmpeg::AVFrame& src) noexcept;

			/**
			 * @brief Opens a new stretch when mean luma jumps.
			 * @param pts Current frame PTS.
			 * @param mean Mean luma of the gray8 probe.
			 */
			void MaybeCut(int64_t pts, double mean) noexcept;

			/**
			 * @brief Votes every stretch. Unstable ones stay skip.
			 */
			void Vote() noexcept;

			/**
			 * @brief Votes one stretch in place.
			 * @param seg Stretch to close.
			 */
			void VoteSeg(Segment& seg) noexcept;

			/**
			 * @brief Stretch that owns @p pts, or null.
			 * @param pts Process-pass PTS.
			 */
			const Segment* Find(int64_t pts) const noexcept;

			/**
			 * @brief Builds the avfilter chain for @ref m_sigma.
			 * @return `fftdnoiz=sigma=:prev=1:next=1:block=32:overlap=0.5`
			 *         for @ref FFmpeg::AVFilterGraph::Ensure.
			 */
			std::string Chain() const noexcept;

			std::optional<double> m_capIn;	///< Caller sigma cap, or empty → 8
			std::vector<Segment> m_seg;	///< Stretches in PTS order
			double m_lastMean;	///< Mean luma of the previous Measure frame
			bool m_haveMean;	///< m_lastMean is valid
			unsigned m_frames;	///< Video units seen in Measure
			bool m_voted;	///< Vote() has run
			double m_sigma;	///< Sigma currently loaded in m_graph
			int m_ran;	///< Stretches that will Process
			int m_skipped;	///< Stretches left untouched
			std::unique_ptr<StormByte::Multimedia::FFmpeg::AVFrame> m_luma;	///< Reused gray8 probe
			std::unique_ptr<StormByte::Multimedia::FFmpeg::AVFilterGraph> m_graph;	///< Reused process graph
	};
}
