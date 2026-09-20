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

#include <StormByte/multimedia/ffmpeg/AVChannelLayout.hxx>
#include <StormByte/multimedia/ffmpeg/AVFilterGraph.hxx>
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/ffmpeg/AVRational.hxx>

#include <cstdio>
#include <cstdint>
#include <utility>

extern "C" {
	#include <libavfilter/avfilter.h>
	#include <libavfilter/buffersink.h>
	#include <libavfilter/buffersrc.h>
	#include <libavutil/error.h>
	#include <libavutil/opt.h>
	#include <libavutil/pixfmt.h>
	#include <libavutil/samplefmt.h>
}

using namespace StormByte::Multimedia;

FFmpeg::AVFilterGraph::AVFilterGraph(::AVFilterGraph* graph) noexcept
: AVPointer(graph) {}

FFmpeg::AVFilterGraph::AVFilterGraph(AVFilterGraph&& other) noexcept
: AVPointer(std::move(other)),
	m_src(other.m_src),
	m_sink(other.m_sink),
	m_audio(other.m_audio),
	m_w(other.m_w),
	m_h(other.m_h),
	m_rate(other.m_rate),
	m_ch(other.m_ch),
	m_fmt(other.m_fmt),
	m_layout(std::move(other.m_layout)),
	m_graph(std::move(other.m_graph)) {
	other.m_src = nullptr;
	other.m_sink = nullptr;
	other.m_audio = false;
	other.m_w = 0;
	other.m_h = 0;
	other.m_rate = 0;
	other.m_ch = 0;
	other.m_fmt = 0;
}

FFmpeg::AVFilterGraph::~AVFilterGraph() noexcept {
	Free();
}

FFmpeg::AVFilterGraph& FFmpeg::AVFilterGraph::operator=(AVFilterGraph&& other) noexcept {
	if (this != &other) {
		Free();
		AVPointer::operator=(std::move(other));
		m_src = other.m_src;
		m_sink = other.m_sink;
		m_audio = other.m_audio;
		m_w = other.m_w;
		m_h = other.m_h;
		m_rate = other.m_rate;
		m_ch = other.m_ch;
		m_fmt = other.m_fmt;
		m_layout = std::move(other.m_layout);
		m_graph = std::move(other.m_graph);
		other.m_src = nullptr;
		other.m_sink = nullptr;
		other.m_audio = false;
		other.m_w = 0;
		other.m_h = 0;
		other.m_rate = 0;
		other.m_ch = 0;
		other.m_fmt = 0;
	}
	return *this;
}

FFmpeg::AVFilterGraph::operator bool() const noexcept {
	return m_ptr != nullptr && m_src != nullptr && m_sink != nullptr;
}

FFmpeg::AVFilterGraph FFmpeg::AVFilterGraph::Open(const AVFrame& src,
	std::string_view graph) noexcept {
	if (!src || graph.empty())
		return AVFilterGraph(nullptr);

	const bool audio = src.Width() <= 0 && src.SampleRate() > 0 && src.Channels() > 0;
	const bool video = src.Width() > 0 && src.Height() > 0;
	if (!audio && !video)
		return AVFilterGraph(nullptr);

	const char* srcName = audio ? "abuffer" : "buffer";
	const char* sinkName = audio ? "abuffersink" : "buffersink";
	const AVFilter* buffersrc = avfilter_get_by_name(srcName);
	const AVFilter* buffersink = avfilter_get_by_name(sinkName);
	if (!buffersrc || !buffersink)
		return AVFilterGraph(nullptr);

	::AVFilterGraph* raw = avfilter_graph_alloc();
	if (!raw)
		return AVFilterGraph(nullptr);

	char args[512];
	std::string layout;
	if (audio) {
		layout = src.ChannelLayout().Describe();
		if (layout.empty())
			return AVFilterGraph(nullptr);
		const char* fmtName = av_get_sample_fmt_name(
			static_cast<AVSampleFormat>(src.Format()));
		if (!fmtName)
			return AVFilterGraph(nullptr);
		std::snprintf(args, sizeof(args),
			"time_base=1/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%s",
			src.SampleRate(), src.SampleRate(), fmtName, layout.c_str());
	} else {
		const auto sar = src.SampleAspectRatio();
		const int sarN = sar.Valid() ? sar.num : 1;
		const int sarD = sar.Valid() ? sar.den : 1;
		std::snprintf(args, sizeof(args),
			"video_size=%dx%d:pix_fmt=%d:time_base=1/1:pixel_aspect=%d/%d",
			src.Width(), src.Height(), src.Format(), sarN, sarD);
	}

	AVFilterContext* in = nullptr;
	AVFilterContext* out = nullptr;
	AVFilterInOut* inputs = nullptr;
	AVFilterInOut* outputs = nullptr;

	auto fail = [&]() {
		avfilter_inout_free(&inputs);
		avfilter_inout_free(&outputs);
		avfilter_graph_free(&raw);
		return AVFilterGraph(nullptr);
	};

	if (avfilter_graph_create_filter(&in, buffersrc, "in", args, nullptr, raw) < 0)
		return fail();
	if (avfilter_graph_create_filter(&out, buffersink, "out", nullptr, nullptr, raw) < 0)
		return fail();

	if (audio) {
		enum AVSampleFormat sfmt[] = {
			static_cast<AVSampleFormat>(src.Format()),
			AV_SAMPLE_FMT_NONE
		};
		if (av_opt_set_bin(out, "sample_fmts", reinterpret_cast<const uint8_t*>(sfmt),
				static_cast<int>(sizeof(sfmt)), AV_OPT_SEARCH_CHILDREN) < 0)
			return fail();
	} else {
		enum AVPixelFormat pix[] = {
			static_cast<AVPixelFormat>(src.Format()),
			AV_PIX_FMT_NONE
		};
		if (av_opt_set_bin(out, "pix_fmts", reinterpret_cast<const uint8_t*>(pix),
				static_cast<int>(sizeof(pix)), AV_OPT_SEARCH_CHILDREN) < 0)
			return fail();
	}

	outputs = avfilter_inout_alloc();
	inputs = avfilter_inout_alloc();
	if (!outputs || !inputs)
		return fail();

	outputs->name = av_strdup("in");
	outputs->filter_ctx = in;
	outputs->pad_idx = 0;
	outputs->next = nullptr;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = out;
	inputs->pad_idx = 0;
	inputs->next = nullptr;

	const std::string chain(graph);
	if (avfilter_graph_parse_ptr(raw, chain.c_str(), &inputs, &outputs, nullptr) < 0)
		return fail();
	if (avfilter_graph_config(raw, nullptr) < 0)
		return fail();

	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);

	AVFilterGraph wrap(raw);
	wrap.m_src = in;
	wrap.m_sink = out;
	wrap.m_audio = audio;
	wrap.m_w = video ? src.Width() : 0;
	wrap.m_h = video ? src.Height() : 0;
	wrap.m_rate = audio ? src.SampleRate() : 0;
	wrap.m_ch = audio ? src.Channels() : 0;
	wrap.m_fmt = src.Format();
	wrap.m_layout = std::move(layout);
	wrap.m_graph = chain;
	return wrap;
}

bool FFmpeg::AVFilterGraph::Ensure(const AVFrame& src, std::string_view graph) noexcept {
	const bool audio = src.Width() <= 0 && src.SampleRate() > 0 && src.Channels() > 0;
	const bool same = m_ptr && m_src && m_sink && m_graph == graph && m_fmt == src.Format()
		&& m_audio == audio
		&& (audio
			? (m_rate == src.SampleRate() && m_ch == src.Channels()
				&& m_layout == src.ChannelLayout().Describe())
			: (m_w == src.Width() && m_h == src.Height()));
	if (same)
		return true;
	AVFilterGraph next = Open(src, graph);
	if (!next)
		return false;
	*this = std::move(next);
	return true;
}

bool FFmpeg::AVFilterGraph::Filter(const AVFrame& src, AVFrame& dst) const noexcept {
	if (!m_ptr || !m_src || !m_sink || !src.Get() || !dst.Get())
		return false;
	if (av_buffersrc_add_frame_flags(m_src, const_cast<::AVFrame*>(src.Get()),
			AV_BUFFERSRC_FLAG_KEEP_REF) < 0)
		return false;
	av_frame_unref(dst.Get());
	const int rc = av_buffersink_get_frame(m_sink, dst.Get());
	if (rc == AVERROR(EAGAIN) || rc == AVERROR_EOF)
		return true;
	return rc >= 0;
}

void FFmpeg::AVFilterGraph::Free() noexcept {
	m_src = nullptr;
	m_sink = nullptr;
	m_audio = false;
	m_w = 0;
	m_h = 0;
	m_rate = 0;
	m_ch = 0;
	m_fmt = 0;
	m_layout.clear();
	m_graph.clear();
	if (m_ptr)
		avfilter_graph_free(&m_ptr);
}

template class StormByte::Multimedia::FFmpeg::AVPointer<::AVFilterGraph>;
