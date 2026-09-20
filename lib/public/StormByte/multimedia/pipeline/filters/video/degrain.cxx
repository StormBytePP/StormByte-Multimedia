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

#include <StormByte/multimedia/pipeline/filters/video/degrain.hxx>
#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/type.hxx>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <format>
#include <map>
#include <utility>

using StormByte::Logger::Level;
using StormByte::Multimedia::Type;
using StormByte::Multimedia::Pipeline::Filter::Video::Degrain;
using FFrame = StormByte::Multimedia::FFmpeg::AVFrame;
using FGraph = StormByte::Multimedia::FFmpeg::AVFilterGraph;

namespace {
	constexpr std::size_t MaxCand = 48;
	constexpr int MinMatches = 3;
	constexpr double MinFrac = 0.25;
	constexpr double Dist = 0.75;
	constexpr double CleanSigma = 0.35;
	constexpr double FlatMax = 6.0;
	constexpr double CutMean = 18.0;
}

Degrain::Degrain(std::shared_ptr<StormByte::Logger::Log> log,
	std::optional<double> sigmaCap) noexcept
	: Filter::ProcessTwoPasses(std::move(log), "degrain"),
	m_capIn(sigmaCap),
	m_lastMean(0.0),
	m_haveMean(false),
	m_frames(0),
	m_voted(false),
	m_sigma(0.0),
	m_ran(0),
	m_skipped(0) {}

enum Type Degrain::Media() const noexcept {
	return Type::Video;
}

void Degrain::Clean() noexcept {
	m_seg.clear();
	m_lastMean = 0.0;
	m_haveMean = false;
	m_frames = 0;
	m_voted = false;
	m_sigma = 0.0;
	m_ran = 0;
	m_skipped = 0;
	m_luma.reset();
	m_graph.reset();
}

void Degrain::Setup() noexcept {
	m_graph.reset();
	m_sigma = 0.0;
}

void Degrain::MaybeCut(int64_t pts, double mean) noexcept {
	if (!m_haveMean) {
		m_haveMean = true;
		m_lastMean = mean;
		Segment s;
		s.firstPts = pts;
		s.lastPts = pts;
		m_seg.push_back(std::move(s));
		return;
	}
	if (std::fabs(mean - m_lastMean) >= CutMean || m_seg.empty()) {
		Segment s;
		s.firstPts = pts;
		s.lastPts = pts;
		m_seg.push_back(std::move(s));
	} else {
		m_seg.back().lastPts = pts;
	}
	m_lastMean = mean;
}

void Degrain::Emit(const FFrame& src) noexcept {
	if (src.Hardware()) {
		Fail("degrain: hardware frame; decode to software first");
		return;
	}
	if (!m_luma) {
		m_luma = std::make_unique<FFrame>();
		m_luma->Format(FFrame::FormatGray8());
	}
	if (!src.ScaleTo(*m_luma, src.Width(), src.Height(),
			FFrame::Resample::Default, FFrame::Scaler::Sws)) {
		Log(Level::Warning, "degrain: luma probe failed");
		return;
	}
	const FFrame& g = *m_luma;
	if (!g.Data(0) || g.Width() < 16 || g.Height() < 16)
		return;

	uint64_t acc = 0;
	const int stepX = std::max(4, g.Width() / 48);
	const int stepY = std::max(4, g.Height() / 48);
	int nMean = 0;
	double sum2 = 0.0;
	int nFlat = 0;
	for (int y = 1; y < g.Height() - 1; y += stepY) {
		const uint8_t* row = g.Data(0) + y * g.Linesize(0);
		const uint8_t* up = g.Data(0) + (y - 1) * g.Linesize(0);
		const uint8_t* dn = g.Data(0) + (y + 1) * g.Linesize(0);
		for (int x = 1; x < g.Width() - 1; x += stepX) {
			const int c = row[x];
			acc += static_cast<uint64_t>(c);
			++nMean;
			const int box = (up[x - 1] + up[x] + up[x + 1]
				+ row[x - 1] + c + row[x + 1]
				+ dn[x - 1] + dn[x] + dn[x + 1]);
			const double mean = static_cast<double>(box) / 9.0;
			double var = 0.0;
			var += std::abs(static_cast<double>(up[x - 1]) - mean);
			var += std::abs(static_cast<double>(up[x]) - mean);
			var += std::abs(static_cast<double>(up[x + 1]) - mean);
			var += std::abs(static_cast<double>(row[x - 1]) - mean);
			var += std::abs(static_cast<double>(c) - mean);
			var += std::abs(static_cast<double>(row[x + 1]) - mean);
			var += std::abs(static_cast<double>(dn[x - 1]) - mean);
			var += std::abs(static_cast<double>(dn[x]) - mean);
			var += std::abs(static_cast<double>(dn[x + 1]) - mean);
			var /= 9.0;
			if (var > FlatMax)
				continue;
			const double r = static_cast<double>(c) - mean;
			sum2 += r * r;
			++nFlat;
		}
	}
	const double mean = nMean
		? static_cast<double>(acc) / static_cast<double>(nMean)
		: 0.0;
	MaybeCut(src.Pts(), mean);
	if (m_seg.empty() || nFlat < 8)
		return;
	const double rms = std::sqrt(sum2 / static_cast<double>(nFlat));
	const double sigma = std::clamp(rms / 4.0, 0.0, 100.0);
	auto& cand = m_seg.back().cand;
	if (cand.size() >= MaxCand)
		cand.erase(cand.begin());
	cand.push_back(sigma);
}

void Degrain::VoteSeg(Segment& seg) noexcept {
	seg.skip = true;
	if (seg.cand.size() < static_cast<std::size_t>(MinMatches))
		return;

	int bestI = -1;
	int bestN = 0;
	const double thr = Dist * Dist;
	for (std::size_t i = 0; i < seg.cand.size(); ++i) {
		int n = 0;
		for (std::size_t j = 0; j < seg.cand.size(); ++j) {
			if (i == j)
				continue;
			const double d = seg.cand[i] - seg.cand[j];
			if (d * d <= thr)
				++n;
		}
		if (n > bestN) {
			bestN = n;
			bestI = static_cast<int>(i);
		}
	}
	const double frac = static_cast<double>(bestN)
		/ static_cast<double>(seg.cand.size());
	if (bestI < 0 || bestN < MinMatches || frac < MinFrac)
		return;

	const double cap = std::clamp(m_capIn.value_or(8.0), 0.0, 100.0);
	seg.sigma = std::clamp(seg.cand[static_cast<std::size_t>(bestI)], 0.0, cap);
	if (seg.sigma < CleanSigma)
		return;
	seg.matches = bestN;
	seg.skip = false;
}

void Degrain::Vote() noexcept {
	m_voted = true;
	m_ran = 0;
	m_skipped = 0;
	for (auto& seg : m_seg) {
		VoteSeg(seg);
		if (seg.skip)
			++m_skipped;
		else
			++m_ran;
	}
	if (m_ran == 0) {
		Log(Level::Warning,
			"degrain: no stable grain stretch; leaving video untouched");
		return;
	}
	Log(Level::Notice, std::format(
		"degrain stretches ran={} skip={} frames={}",
		m_ran, m_skipped, m_frames));
}

const Degrain::Segment* Degrain::Find(int64_t pts) const noexcept {
	for (const auto& seg : m_seg) {
		const int64_t a = std::min(seg.firstPts, seg.lastPts);
		const int64_t b = std::max(seg.firstPts, seg.lastPts);
		if (pts >= a && pts <= b)
			return &seg;
	}
	if (!m_seg.empty() && pts >= std::max(m_seg.back().firstPts, m_seg.back().lastPts))
		return &m_seg.back();
	return nullptr;
}

std::string Degrain::Chain() const noexcept {
	return std::format(
		"fftdnoiz=sigma={}:prev=1:next=1:block=32:overlap=0.5",
		m_sigma);
}

void Degrain::Measure(const Pipeline::Frame& frame) noexcept {
	if (m_voted)
		return;
	if (frame.Type() != Type::Video)
		return;
	const FFrame& src = AVFrame();
	if (!src || src.Width() <= 0 || src.Height() <= 0) {
		Log(Level::Warning, "degrain: frame has no picture");
		return;
	}
	Emit(src);
	++m_frames;
}

void Degrain::Process(const Pipeline::Frame& frame) noexcept {
	if (frame.Type() != Type::Video)
		return;
	if (!m_voted)
		Vote();

	const FFrame& src = AVFrame();
	if (!src || src.Width() <= 0 || src.Height() <= 0) {
		Log(Level::Warning, "degrain: frame has no picture");
		return;
	}
	const Segment* seg = Find(src.Pts());
	if (!seg || seg->skip)
		return;
	if (src.Hardware()) {
		Fail("degrain: hardware frame; decode to software first");
		return;
	}

	if (!m_graph || std::fabs(m_sigma - seg->sigma) > 1e-6) {
		m_sigma = seg->sigma;
		m_graph.reset();
		FGraph opened = FGraph::Open(src, Chain());
		if (!opened) {
			Fail("degrain: AVFilterGraph::Open failed");
			return;
		}
		m_graph = std::make_unique<FGraph>(std::move(opened));
	} else if (!m_graph->Ensure(src, Chain())) {
		Fail("degrain: AVFilterGraph::Ensure failed");
		return;
	}

	FFrame out;
	if (!m_graph->Filter(src, out)) {
		Fail("degrain: AVFilterGraph::Filter failed");
		return;
	}
	if (out.Width() <= 0 || out.Height() <= 0 || out.Format() == FFrame::FormatNone()) {
		Log(Level::LowLevel, std::format(
			"degrain wait {}x{} pts={}",
			src.Width(), src.Height(), src.Pts()));
		return;
	}

	out.ColorRange(src.ColorRange());
	out.ColorSpace(src.ColorSpace());
	out.ColorPrimaries(src.ColorPrimaries());
	out.ColorTransfer(src.ColorTransfer());
	out.ChromaLocation(src.ChromaLocation());
	out.SampleAspectRatio(src.SampleAspectRatio());

	Log(Level::LowLevel, std::format(
		"degrain {}x{} sigma={:.2f} pts={}",
		out.Width(), out.Height(), m_sigma, out.Pts()));
	Save(std::move(out));
}

void Degrain::Eof() noexcept {
	if (!m_voted) {
		Vote();
		return;
	}
	if (!m_graph)
		return;
	for (;;) {
		FFrame out;
		if (!m_graph->Flush(out)) {
			Fail("degrain: AVFilterGraph::Flush failed");
			break;
		}
		if (out.Width() <= 0 || out.Height() <= 0 || out.Format() == FFrame::FormatNone())
			break;
		Save(std::move(out));
	}
	m_graph.reset();
}

class StormByte::Multimedia::Pipeline::Filter::Report Degrain::Report() const noexcept {
	if (!m_voted)
		return { Filter::Report::Status::Failed, {} };
	std::map<std::string, std::string> data;
	data.emplace("stretches", std::to_string(m_seg.size()));
	data.emplace("ran", std::to_string(m_ran));
	data.emplace("skipped", std::to_string(m_skipped));
	data.emplace("frames", std::to_string(m_frames));
	data.emplace("sigma", std::format("{:.3f}", m_sigma));
	return { Filter::Report::Status::Ok, std::move(data) };
}
