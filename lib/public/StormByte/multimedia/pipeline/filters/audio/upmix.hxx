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
#include <StormByte/multimedia/property/channel_layout.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>
#include <string>
#include <string_view>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Audio
 * @brief Audio process filters.
 */
namespace StormByte::Multimedia::Pipeline::Filter::Audio {
	/**
	 * @class Upmix
	 * @brief Widen channel count via libavfilter `surround`, then `aformat`, then `pan`. Process leaf.
	 *
	 * Attach with @c job.Audio(in).Filter<Upmix>(log, layout).
	 *
	 * @par Algorithm
	 * Compare @ref Property::ChannelCount of the constructor
	 * target with @ref FFmpeg::AVFrame::Channels of the source:
	 * fewer source channels than wanted opens a graph; equal
	 * count is a no-op (no graph, no @ref Filter::FFmpeg::Save);
	 * more source channels than wanted is @ref Filter::FFmpeg::Fail
	 * — that job belongs to @ref Downmix. Stereo→5.1 is a valid
	 * upmix. Hardware does not apply to audio.
	 *
	 * @par Quality
	 * This is not a 1:1 5.1 / 7.1 room recording. The first
	 * graph is libavfilter @c surround with LFE enabled when
	 * the target has an LFE speaker (`lfe=1:lfe_mode=add`,
	 * 128–256 Hz band). That extracts a real sub band from
	 * the source instead of leaving LFE silent. If `surround`
	 * cannot open for this pair of layouts, @c aformat copies
	 * what swr can map. Last fallback is @c pan: fronts kept,
	 * LFE = @c 0.5*FL+0.5*FR so the sub is never empty when
	 * the target has LFE. Bass is best-effort, not a remixer
	 * with extra pads.
	 *
	 * @par Names
	 * FFmpeg tokens: mono, stereo, 2.1, 3.0, 4.0, quad, 5.0,
	 * 5.1, 6.1, 7.1, 7.1(wide), octagonal, 22.2.
	 * @ref Property::ChannelLayout::Unknown has no token and Fails.
	 * Input layout name comes from
	 * @ref FFmpeg::AVChannelLayout::Describe, not @c Get().
	 *
	 * @par Mutation
	 * @ref Filter::FFmpeg::Save of the abuffersink frame.
	 * @ref Backend::Pipeline::Frame::Put then
	 * @ref Backend::Pipeline::Frame::BindProperties rewrites
	 * @ref Pipeline::Frame::Audio from the handle. Empty sink
	 * with a successful Filter is EAGAIN: log wait and return.
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::Process
	 * @see StormByte::Multimedia::Pipeline::Filter::Audio::Downmix
	 * @see StormByte::Multimedia::FFmpeg::AVFilterGraph
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Upmix: public Filter::Process {
		public:
			/**
			 * @brief Upmix toward @p target.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param target Destination speaker layout (not a fixed 5.1).
			 */
			Upmix(std::shared_ptr<StormByte::Logger::Log> log,
				Property::ChannelLayout target) noexcept;

			/**
			 * @brief Copy is not allowed. The graph is bound to one tube.
			 */
			Upmix(const Upmix& other) = delete;

			/**
			 * @brief Move is not allowed. The tube owns the mounted leaf.
			 */
			Upmix(Upmix&& other) noexcept = delete;

			/**
			 * @brief Drops the cached graph.
			 */
			~Upmix() noexcept override = default;

			/**
			 * @brief Copy assignment is not allowed.
			 */
			Upmix& operator=(const Upmix& other) = delete;

			/**
			 * @brief Move assignment is not allowed.
			 */
			Upmix& operator=(Upmix&& other) noexcept = delete;

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
			 * @brief Pushes one audio frame through surround/aformat/pan and Save.
			 * @param frame Current pipeline unit. Non-audio is ignored.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Drains frames still held by the graph, if any.
			 */
			void Eof() noexcept override;

		private:
			/**
			 * @brief How the current graph was built.
			 */
			enum class Mode: std::uint8_t {
				None,		///< No graph yet
				Surround,	///< `surround` with LFE band
				Aformat,	///< `aformat=channel_layouts=`
				Pan			///< `pan` fronts + summed LFE
			};

			/**
			 * @brief FFmpeg layout token for @ref m_target.
			 * @return Token, or empty when the layout is unknown.
			 */
			std::string_view LayoutName() const noexcept;

			/**
			 * @brief Whether @ref m_target includes an LFE speaker.
			 * @return true for 2.1 / 5.1 / 6.1 / 7.1 / 7.1(wide) / 22.2.
			 */
			bool TargetHasLfe() const noexcept;

			/**
			 * @brief Preferred chain: `surround=chl_in=…:chl_out=…:lfe=…`.
			 * @param src Model frame (Describe for chl_in).
			 * @return Filterchain, or empty if @ref LayoutName is empty.
			 */
			std::string SurroundChain(const StormByte::Multimedia::FFmpeg::AVFrame& src) const noexcept;

			/**
			 * @brief Second chain: `aformat=channel_layouts=…`.
			 * @return Filterchain, or empty if @ref LayoutName is empty.
			 */
			std::string AformatChain() const noexcept;

			/**
			 * @brief Last chain: keep FL/FR, synthesize LFE from the sum.
			 * @return Filterchain, or empty if @ref LayoutName is empty.
			 */
			std::string PanChain() const noexcept;

			/**
			 * @brief Opens or reuses @ref m_graph for @p src and @p chain.
			 * @param src Model audio frame.
			 * @param chain avfilter filterchain.
			 * @return false if the graph could not be (re)opened.
			 */
			bool EnsureGraph(const StormByte::Multimedia::FFmpeg::AVFrame& src,
				std::string_view chain) noexcept;

			Property::ChannelLayout m_target;	///< Destination layout
			std::unique_ptr<StormByte::Multimedia::FFmpeg::AVFilterGraph> m_graph;	///< Reused graph
			Mode m_mode = Mode::None;			///< Active fallback step
	};
}
