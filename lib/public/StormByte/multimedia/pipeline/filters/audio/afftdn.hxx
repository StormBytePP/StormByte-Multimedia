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
 * @namespace StormByte::Multimedia::Pipeline::Filter::Audio
 * @brief Audio process filters.
 */
namespace StormByte::Multimedia::Pipeline::Filter::Audio {
	/**
	 * @class Afftdn
	 * @brief FFT denoise via libavfilter `afftdn`. Process leaf.
	 *
	 * Attach with @c job.Audio(in).Filter<Afftdn>(log).
	 *
	 * @par Algorithm
	 * Graph is @c afftdn=nr=:nf=:tn=:nt=w:om=o. Empty @a nr is
	 * 12 dB (FFmpeg default, range 0.01–97). Empty @a nf is
	 * −50 dB (range −80…−20). Empty @a trackNoise is off
	 * (FFmpeg default). Noise type stays white; there is no
	 * custom @c bn profile and no @c sample_noise command
	 * window. Hardware does not apply to audio. This leaf
	 * does not Hold; the graph owns FFT overlap.
	 *
	 * @par Mutation
	 * @ref Filter::FFmpeg::Save of the abuffersink frame.
	 * Empty sink with a successful Filter is EAGAIN: log
	 * wait and return. @ref Eof flushes.
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::Process
	 * @see StormByte::Multimedia::FFmpeg::AVFilterGraph
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Afftdn: public Filter::Process {
		public:
			/**
			 * @brief FFT denoise (`afftdn`).
			 * @param log Shared logger. Empty pointer means no log.
			 * @param nr Noise reduction in dB. Empty → 12.
			 * @param nf Noise floor in dB. Empty → −50.
			 * @param trackNoise Enable floor tracking (`tn`). Empty → false.
			 */
			Afftdn(std::shared_ptr<StormByte::Logger::Log> log,
				std::optional<double> nr = {},
				std::optional<double> nf = {},
				std::optional<bool> trackNoise = {}) noexcept;

			/**
			 * @brief Copy is not allowed. The graph is bound to one tube.
			 */
			Afftdn(const Afftdn& other) = delete;

			/**
			 * @brief Move is not allowed. The tube owns the mounted leaf.
			 */
			Afftdn(Afftdn&& other) noexcept = delete;

			/**
			 * @brief Drops the cached graph.
			 */
			~Afftdn() noexcept override = default;

			/**
			 * @brief Copy assignment is not allowed.
			 */
			Afftdn& operator=(const Afftdn& other) = delete;

			/**
			 * @brief Move assignment is not allowed.
			 */
			Afftdn& operator=(Afftdn&& other) noexcept = delete;

			/**
			 * @brief Media this filter handles.
			 * @return Audio.
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
			 * @brief Pushes one audio frame through `afftdn` and Save.
			 * @param frame Current pipeline unit. Non-audio is ignored.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Drains frames still held by the graph, if any.
			 */
			void Eof() noexcept override;

		private:
			/**
			 * @brief Builds the avfilter chain.
			 * @return `afftdn=…` for @ref FFmpeg::AVFilterGraph::Ensure.
			 */
			std::string Chain() const noexcept;

			std::optional<double> m_nrIn;		///< Caller nr, or empty
			std::optional<double> m_nfIn;		///< Caller nf, or empty
			std::optional<bool> m_trackIn;		///< Caller tn, or empty
			std::unique_ptr<StormByte::Multimedia::FFmpeg::AVFilterGraph> m_graph;	///< Reused graph
	};
}
