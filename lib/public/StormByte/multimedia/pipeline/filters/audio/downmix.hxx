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
	 * @class Downmix
	 * @brief Reduce channel count via libavfilter `aformat` (then `pan`). Process leaf.
	 *
	 * Attach with @c job.Audio(in).Filter<Downmix>(log, layout).
	 *
	 * @par Algorithm
	 * Compare @ref Property::ChannelCount of the constructor
	 * target with @ref FFmpeg::AVFrame::Channels of the source:
	 * more source channels than wanted opens
	 * @c aformat=channel_layouts=&lt;ffmpeg name&gt;; equal count
	 * is a no-op (no graph, no @ref Filter::FFmpeg::Save);
	 * fewer source channels than wanted is @ref Filter::FFmpeg::Fail
	 * (this leaf does not invent speakers). 7.1→5.1 is a valid
	 * downmix. Hardware does not apply to audio.
	 *
	 * @par LFE
	 * `aformat` is tried first so a standard layout keeps its
	 * LFE when the target has one. If that graph cannot open,
	 * a second graph @c pan=&lt;name&gt;|FL=FL+0.707*LFE|FR=FR+0.707*LFE
	 * folds LFE into the front pair. That fallback is not a
	 * 5.1 / 7.1 room mix: surrounds stay as `aformat`/`pan`
	 * map them; LFE energy is only the best effort fold into
	 * FL/FR. Same-count input is still a no-op.
	 *
	 * @par Names
	 * FFmpeg `aformat` tokens: mono, stereo, 2.1, 3.0, 4.0,
	 * quad, 5.0, 5.1, 6.1, 7.1, 7.1(wide), octagonal, 22.2.
	 * @ref Property::ChannelLayout::Unknown has no token and Fails.
	 *
	 * @par Mutation
	 * @ref Filter::FFmpeg::Save of the abuffersink frame.
	 * @ref Backend::Pipeline::Frame::Put then
	 * @ref Backend::Pipeline::Frame::BindProperties rewrites
	 * @ref Pipeline::Frame::Audio (rate, channels, layout)
	 * from the handle mask. Empty sink with a successful
	 * Filter is EAGAIN: log wait and return.
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::Process
	 * @see StormByte::Multimedia::FFmpeg::AVFilterGraph
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Downmix: public Filter::Process {
		public:
			/**
			 * @brief Downmix toward @p target.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param target Destination speaker layout (not a fixed stereo).
			 */
			Downmix(std::shared_ptr<StormByte::Logger::Log> log,
				Property::ChannelLayout target) noexcept;

			/**
			 * @brief Copy is not allowed. The graph is bound to one tube.
			 */
			Downmix(const Downmix& other) = delete;

			/**
			 * @brief Move is not allowed. The tube owns the mounted leaf.
			 */
			Downmix(Downmix&& other) noexcept = delete;

			/**
			 * @brief Drops the cached graph.
			 */
			~Downmix() noexcept override = default;

			/**
			 * @brief Copy assignment is not allowed.
			 */
			Downmix& operator=(const Downmix& other) = delete;

			/**
			 * @brief Move assignment is not allowed.
			 */
			Downmix& operator=(Downmix&& other) noexcept = delete;

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
			 * @brief Pushes one audio frame through `aformat`/`pan` and Save.
			 * @param frame Current pipeline unit. Non-audio is ignored.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Drains frames still held by the graph, if any.
			 */
			void Eof() noexcept override;

		private:
			/**
			 * @brief FFmpeg `aformat` / `pan` layout token for @ref m_target.
			 * @return Token, or empty when the layout is unknown.
			 */
			std::string_view LayoutName() const noexcept;

			/**
			 * @brief Preferred chain: `aformat=channel_layouts=…`.
			 * @return Filterchain, or empty if @ref LayoutName is empty.
			 */
			std::string AformatChain() const noexcept;

			/**
			 * @brief Fallback chain: fold LFE into FL/FR via `pan`.
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
			bool m_pan = false;					///< true after LFE pan fallback
	};
}
