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

#include <StormByte/multimedia/ffmpeg/AVPointer.hxx>
#include <StormByte/multimedia/ffmpeg/fwd.hxx>
#include <StormByte/multimedia/visibility.h>

#include <string>
#include <string_view>

/**
 * @namespace StormByte::Multimedia::FFmpeg
 * @brief RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::FFmpeg {
	class AVFrame;

	/**
	 * @class AVFilterGraph
	 * @brief RAII `AVFilterGraph` with a single video in/out.
	 *
	 * Builds `buffer → <graph> → buffersink`. @p graph is an
	 * avfilter filterchain (e.g. @c "bm3d=sigma=3:estim=basic").
	 * One input pad only; filters that need a @c ref pad fail
	 * @ref Open.
	 *
	 * Reuse the same wrapper across frames of the same size,
	 * format and chain via @ref Ensure. Do not call av_*.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC AVFilterGraph: public AVPointer<::AVFilterGraph> {
		public:
			/**
			 * @brief Move constructor. Transfers the graph.
			 * @param other Source graph; left empty.
			 */
			AVFilterGraph(AVFilterGraph&& other) noexcept;

			/**
			 * @brief Destructor. Frees the graph and its filters.
			 */
			~AVFilterGraph() noexcept override;

			/**
			 * @brief Move assignment. Frees *this, then takes @p other.
			 * @param other Source graph; left empty.
			 * @return *this.
			 */
			AVFilterGraph& operator=(AVFilterGraph&& other) noexcept;

			AVFilterGraph(const AVFilterGraph&) = delete;
			AVFilterGraph& operator=(const AVFilterGraph&) = delete;

			/**
			 * @brief Whether the graph is configured.
			 * @return true if @ref Filter can run.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @brief Builds a graph for @p src's geometry. Empty wrapper on failure.
			 * @param src Model frame (size, format, SAR). Planes may be empty.
			 * @param graph Filterchain after @c buffer, before @c buffersink.
			 * @return Open graph, or empty on failure.
			 */
			static AVFilterGraph Open(const AVFrame& src, std::string_view graph) noexcept;

			/**
			 * @brief Rebuilds if size, format or chain changed.
			 * @param src Model frame.
			 * @param graph Filterchain.
			 * @return false if the graph could not be (re)opened.
			 */
			bool Ensure(const AVFrame& src, std::string_view graph) noexcept;

			/**
			 * @brief Pushes @p src and pulls into @p dst (`KEEP_REF` on the source).
			 * @param src Source frame with buffers.
			 * @param dst Destination; unreferenced then filled by buffersink.
			 * @return false on failure.
			 */
			bool Filter(const AVFrame& src, AVFrame& dst) const noexcept;

		private:
			/**
			 * @brief Adopts an allocated graph.
			 * @param graph libavfilter graph, or nullptr.
			 */
			explicit AVFilterGraph(::AVFilterGraph* graph) noexcept;

			/**
			 * @brief Deleted. Use @ref Open.
			 */
			AVFilterGraph() = delete;

			/**
			 * @brief Frees the graph (`avfilter_graph_free`).
			 */
			void Free() noexcept override;

			using AVPointer<::AVFilterGraph>::Get;

			::AVFilterContext* m_src = nullptr;	///< buffer (owned by the graph)
			::AVFilterContext* m_sink = nullptr;	///< buffersink (owned by the graph)
			int m_w = 0;							///< Cached source width
			int m_h = 0;							///< Cached source height
			int m_fmt = 0;							///< Cached source format
			std::string m_graph;					///< Cached filterchain
	};

	extern template class STORMBYTE_MULTIMEDIA_PUBLIC AVPointer<::AVFilterGraph>;
}
