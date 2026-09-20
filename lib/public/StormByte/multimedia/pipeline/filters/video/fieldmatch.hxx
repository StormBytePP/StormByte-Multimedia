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
 * Attach with @c job.Video(in).Filter<Fieldmatch>(log).Filter<Decimate>(log).
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @class Fieldmatch
	 * @brief Inverse-telecine field matcher via libavfilter `fieldmatch`. Process leaf.
	 *
	 * @par Algorithm
	 * Reconstructs progressive frames from a 3:2 telecine by matching
	 * fields across neighbouring pictures (p/c/n, optional u/b). It
	 * does **not** drop the duplicate. A complete IVTC is this leaf
	 * followed by @ref Decimate on the same stretch.
	 *
	 * The graph owns the look-ahead; this leaf does not Hold. Early
	 * frames may not leave @c buffersink (`EAGAIN`). @ref Process
	 * then returns without @ref Filter::FFmpeg::Save. Drain the tail
	 * in @ref Eof via @c AVFilterGraph::Flush.
	 *
	 * @par What it is not
	 * Not a deinterlacer. Do not stack @ref Bwdif or @ref Yadif
	 * *before* this leaf on a hard telecine: they destroy the field
	 * relationship fieldmatch needs. A residual comb after match
	 * can go through Yadif @c onlyInterlaced, then Decimate.
	 *
	 * Native 23.976/24 progressive film must not use this pair.
	 *
	 * @par HDR
	 * Geometry is unchanged. Primaries, transfer, range, chroma
	 * siting and SAR of the source are copied onto the sink frame.
	 *
	 * @par Defaults
	 * Empty arguments: @c order=auto, @c mode=pc_n, @c combmatch=sc.
	 * Same as FFmpeg. Dirty edits: pass @c combmatch=full.
	 *
	 * @par Mutation
	 * @ref Filter::FFmpeg::Save of the buffersink frame.
	 * Hardware frames Fail. Non-video units are ignored.
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::Video::Decimate
	 * @see StormByte::Multimedia::FFmpeg::AVFilterGraph
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Fieldmatch: public Filter::Process {
		public:
			/**
			 * @brief Field matcher (`fieldmatch`).
			 * @param log Shared logger. Empty pointer means no log.
			 * @param order Field order: @c auto, @c tff or @c bff. Empty → @c auto.
			 * @param mode Match strategy: @c pc, @c pc_n, @c pc_u, @c pc_n_ub,
			 *        @c pcn or @c pcn_ub. Empty → @c pc_n.
			 * @param combmatch Comb score use: @c none, @c sc or @c full.
			 *        Empty → @c sc.
			 */
			Fieldmatch(std::shared_ptr<StormByte::Logger::Log> log,
				std::optional<std::string> order = {},
				std::optional<std::string> mode = {},
				std::optional<std::string> combmatch = {}) noexcept;

			/**
			 * @brief Copy is not allowed. The graph is bound to one tube.
			 */
			Fieldmatch(const Fieldmatch& other) = delete;

			/**
			 * @brief Move is not allowed. The tube owns the mounted leaf.
			 */
			Fieldmatch(Fieldmatch&& other) noexcept = delete;

			/**
			 * @brief Drops the cached graph.
			 */
			~Fieldmatch() noexcept override = default;

			/**
			 * @brief Copy assignment is not allowed.
			 */
			Fieldmatch& operator=(const Fieldmatch& other) = delete;

			/**
			 * @brief Move assignment is not allowed.
			 */
			Fieldmatch& operator=(Fieldmatch&& other) noexcept = delete;

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
			 * @brief Pushes one video frame through `fieldmatch` and Save.
			 * @param frame Current pipeline unit. Non-video is ignored.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Drains frames still held by the match window.
			 */
			void Eof() noexcept override;

		private:
			/**
			 * @brief Builds the avfilter chain.
			 * @return `fieldmatch=…` for @ref FFmpeg::AVFilterGraph::Ensure.
			 */
			std::string Chain() const noexcept;

			std::optional<std::string> m_orderIn;		///< Caller order, or empty
			std::optional<std::string> m_modeIn;		///< Caller mode, or empty
			std::optional<std::string> m_combIn;		///< Caller combmatch, or empty
			std::unique_ptr<StormByte::Multimedia::FFmpeg::AVFilterGraph> m_graph;	///< Reused graph
	};
}
