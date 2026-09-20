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

/**
 * Tunables. Change these — not the vote shape — when a new title
 * disagrees with Harry Potter / Privet / troll.
 *
 * Ring: we only ever keep 11 YUV420P copies. ScoreCenter runs on the
 * slot that already has 5 past and (once the ring is full) 5 future
 * pictures. The first and last handful of frames therefore see a
 * shorter window; those frames are not the ones the eye parks on.
 *
 * Bands: grain is more visible in dark and mid luma than in specular
 * highlights. raw is a weighted RMS of the temporal residual in those
 * two bands only. Live/skin are hints for “do not go to the wall cap”.
 */
namespace {
	/** Past/future pictures used as temporal neighbours. */
	constexpr int RingSpan = 5;
	/** 5 + current + 5. Hard cap on pixel RAM. */
	constexpr int RingMax = RingSpan * 2 + 1;
	/** Luma ≤ this counts as the dark residual band (night grain). */
	constexpr double DarkHi = 56.0;
	/** Luma ≤ this (and > DarkHi) counts as the mid residual band. */
	constexpr double MidHi = 110.0;
	/** Weight of dark RMS inside raw. Night grain is the contract. */
	constexpr double GainDark = 0.70;
	/** Weight of mid RMS inside raw. Walls / torch sit here. */
	constexpr double GainMid = 0.55;
	/** |Y now − Y neighbour| above this is motion, not grain. */
	constexpr double MotionPix = 10.0;
	/** Neighbour frame mean must stay within this or the lighting changed. */
	constexpr double NeighMean = 12.0;
	/** |frame mean − local-median mean| to flag a lightning / cut flash. */
	constexpr double FlashDelta = 22.0;
	/** Half-window (in rows) for that local median. */
	constexpr int FlashWin = 8;
	/** Persist a new stretch when |mean − anchor| stays ≥ this. */
	constexpr double CutMean = 28.0;
	/** Or when |raw − rawAnchor| stays ≥ this (slow fade, same mean). */
	constexpr double CutRaw = 2.4;
	/** Consecutive jump rows required before the cut is real. */
	constexpr int CutPersist = 8;
	/** Spatial |box5 − box15| that counts as structure, not flat grain. */
	constexpr double LiveBand = 4.5;
	/** That structure must also hold vs the neighbour box5 (stable edge). */
	constexpr double LiveHold = 4.0;
	/** Median fLive that marks the stretch “alive”. */
	constexpr double LiveNeed = 0.10;
	/** Median fSkin that marks the stretch “alive”. */
	constexpr double SkinNeed = 0.08;
	/** Skin box, Y low. Below this is shadow / crushed black. */
	constexpr int SkinY0 = 50;
	/** Skin box, Y high. Above this is highlight / wall paint. */
	constexpr int SkinY1 = 180;
	/** Skin box, U upper (toward magenta/red, away from cyan walls). */
	constexpr int SkinUMax = 128;
	/** Skin box, V lower (red-ish). */
	constexpr int SkinVMin = 133;
	/** Skin box, V−U minimum so neutral grey is out. */
	constexpr int SkinVU = 12;
	/** Median fFlat that counts as a wall / floor, not a face CU. */
	constexpr double FlatNeed = 0.22;
	/** Median mot that forces the close (conservative) sigma. */
	constexpr double MotBusy = 0.22;
	/** Weakest sigma we will ever send to fftdnoiz. */
	constexpr double SigmaFloor = 1.2;
	/** Night floor. Crushed blacks still get this; they do not skip. */
	constexpr double SigmaNight = 3.0;
	/** mean ≤ this is night, regardless of flat (flat ignores crushed Y). */
	constexpr double NightMean = 38.0;
	/** Faces / busy motion. Below film-default fftdnoiz (1.0) would do nothing. */
	constexpr double SigmaClose = 2.0;
	/** Alive and flat at once (street + people). */
	constexpr double SigmaMix = 2.6;
	/** Hard cap on a dead flat wall. */
	constexpr double SigmaWall = 4.0;
	/** Mid/bright raw below this and not night → skip (already clean). */
	constexpr double CleanSigma = 0.20;
	/** Need this many dark+mid samples or raw is not trusted (→ 0). */
	constexpr int MinBand = 24;
}

Degrain::Degrain(std::shared_ptr<StormByte::Logger::Log> log,
	std::optional<double> sigmaCap) noexcept
	: Filter::ProcessTwoPasses(std::move(log), "degrain"),
	m_capIn(sigmaCap),
	m_frames(0),
	m_voted(false),
	m_sigma(0.0),
	m_sigmaMin(0.0),
	m_sigmaP50(0.0),
	m_sigmaMax(0.0),
	m_ran(0),
	m_skipped(0) {}

enum Type Degrain::Media() const noexcept {
	return Type::Video;
}

void Degrain::Clean() noexcept {
	m_ring.clear();
	m_row.clear();
	m_seg.clear();
	m_frames = 0;
	m_voted = false;
	m_sigma = 0.0;
	m_sigmaMin = 0.0;
	m_sigmaP50 = 0.0;
	m_sigmaMax = 0.0;
	m_ran = 0;
	m_skipped = 0;
	m_graph.reset();
}

void Degrain::Setup() noexcept {
	m_graph.reset();
	m_sigma = 0.0;
}

void Degrain::ScoreCenter(std::size_t idx) noexcept {
	if (idx >= m_ring.size() || !m_ring[idx].pic)
		return;
	const int64_t pts = m_ring[idx].pts;
	for (const auto& r : m_row)
		if (r.pts == pts)
			return;
	const FFrame& cur = *m_ring[idx].pic;
	if (!cur.Data(0) || cur.Width() < 32 || cur.Height() < 32)
		return;

	const auto pixY = [&](const FFrame& g, int x, int y) noexcept {
		return static_cast<int>(g.Data(0)[y * g.Linesize(0) + x]);
	};
	const auto box = [&](const FFrame& g, int x, int y, int r) noexcept {
		int s = 0, n = 0;
		const int x0 = std::max(0, x - r);
		const int y0 = std::max(0, y - r);
		const int x1 = std::min(g.Width() - 1, x + r);
		const int y1 = std::min(g.Height() - 1, y + r);
		for (int yy = y0; yy <= y1; ++yy)
			for (int xx = x0; xx <= x1; ++xx) {
				s += pixY(g, xx, yy);
				++n;
			}
		return n ? static_cast<double>(s) / static_cast<double>(n) : 0.0;
	};

	const bool chroma = cur.Data(1) && cur.Data(2)
		&& cur.Linesize(1) > 0 && cur.Linesize(2) > 0;
	const int cw = std::max(1, cur.Width() / 2);
	const int ch = std::max(1, cur.Height() / 2);

	int nMean = 0, nLive = 0, nSkin = 0, nSeen = 0, nMot = 0, nFlat = 0;
	int nDark = 0, nMid = 0;
	double dark2 = 0.0, mid2 = 0.0;
	uint64_t acc = 0;
	int hist[64]{};
	const int stepX = std::max(3, cur.Width() / 96);
	const int stepY = std::max(3, cur.Height() / 96);
	const double cMean = m_ring[idx].mean;

	/*
	* Grid walk. Each sample:
	*  - accumulates luma / histogram (mean, p10, p90);
	*  - measures spatial grain-vs-structure (box5 vs box15);
	*  - looks for a neighbour whose pixel is close enough to be
	*    “same surface, different grain” (e). No such neighbour
	*    is motion / a cut — counted in mot, not in raw;
	*  - live = structure that also holds in time (hair, edges);
	*  - skin = 4:2:0 UV box, only a hint;
	*  - flat = almost no spatial band, typical of a painted wall.
	*/
	for (int y = 8; y < cur.Height() - 8; y += stepY) {
		for (int x = 8; x < cur.Width() - 8; x += stepX) {
			const int c = pixY(cur, x, y);
			acc += static_cast<uint64_t>(c);
			++nMean;
			hist[std::clamp(c / 4, 0, 63)]++;
			++nSeen;
			const double b5 = box(cur, x, y, 2);
			const double b15 = box(cur, x, y, 7);
			const double band = std::fabs(b5 - b15);
			double e = 255.0;
			int got = 0;
			for (int k = -RingSpan; k <= RingSpan; ++k) {
				if (k == 0)
					continue;
				const int j = static_cast<int>(idx) + k;
				if (j < 0 || j >= static_cast<int>(m_ring.size()) || !m_ring[static_cast<std::size_t>(j)].pic)
					continue;
				if (std::fabs(m_ring[static_cast<std::size_t>(j)].mean - cMean) > NeighMean)
					continue;
				const FFrame& nb = *m_ring[static_cast<std::size_t>(j)].pic;
				if (nb.Width() != cur.Width() || nb.Height() != cur.Height() || !nb.Data(0))
					continue;
				const double te = std::fabs(static_cast<double>(c - pixY(nb, x, y)));
				if (te > MotionPix)
					continue;
				e = std::min(e, te);
				++got;
			}
			if (!got) {
				++nMot;
				continue;
			}
			double holdN = 255.0;
			for (int k = -RingSpan; k <= RingSpan; ++k) {
				if (k == 0)
					continue;
				const int j = static_cast<int>(idx) + k;
				if (j < 0 || j >= static_cast<int>(m_ring.size()) || !m_ring[static_cast<std::size_t>(j)].pic)
					continue;
				const FFrame& nb = *m_ring[static_cast<std::size_t>(j)].pic;
				if (nb.Width() != cur.Width() || nb.Height() != cur.Height() || !nb.Data(0))
					continue;
				holdN = std::min(holdN, std::fabs(b5 - box(nb, x, y, 2)));
			}
			if (band >= LiveBand && holdN <= LiveHold)
				++nLive;
			if (chroma && c >= SkinY0 && c <= SkinY1) {
				const int ux = std::clamp(x / 2, 0, cw - 1);
				const int uy = std::clamp(y / 2, 0, ch - 1);
				const int u = static_cast<int>(cur.Data(1)[uy * cur.Linesize(1) + ux]);
				const int v = static_cast<int>(cur.Data(2)[uy * cur.Linesize(2) + ux]);
				if (u <= SkinUMax && v >= SkinVMin && (v - u) >= SkinVU)
					++nSkin;
			}
			if (c <= static_cast<int>(DarkHi)) {
				dark2 += e * e;
				++nDark;
			} else if (c <= static_cast<int>(MidHi)) {
				mid2 += e * e;
				++nMid;
			}
			if (band < 2.0)
				++nFlat;
		}
	}

	Row row;
	row.pts = pts;
	row.mean = nMean ? static_cast<double>(acc) / static_cast<double>(nMean) : 0.0;
	int seenH = 0, p10i = 0, p90i = 0;
	const int need10 = std::max(1, nMean / 10);
	const int need90 = std::max(1, (nMean * 9) / 10);
	for (int b = 0; b < 64; ++b) {
		seenH += hist[b];
		if (!p10i && seenH >= need10)
			p10i = b * 4 + 2;
		if (!p90i && seenH >= need90)
			p90i = b * 4 + 2;
	}
	row.p10 = static_cast<double>(p10i);
	row.p90 = static_cast<double>(p90i);
	row.dark = nDark ? std::sqrt(dark2 / static_cast<double>(nDark)) : 0.0;
	row.mid = nMid ? std::sqrt(mid2 / static_cast<double>(nMid)) : 0.0;
	row.fLive = nSeen ? static_cast<double>(nLive) / static_cast<double>(nSeen) : 0.0;
	row.fSkin = nSeen ? static_cast<double>(nSkin) / static_cast<double>(nSeen) : 0.0;
	row.fFlat = nSeen ? static_cast<double>(nFlat) / static_cast<double>(nSeen) : 0.0;
	row.mot = nSeen ? static_cast<double>(nMot) / static_cast<double>(nSeen) : 0.0;
	row.raw = (nDark + nMid) >= MinBand
		? GainDark * row.dark + GainMid * row.mid
		: 0.0;
	Log(Level::Debug, std::format(
		"row pts={} mean={:.1f} live={:.2f} skin={:.2f} flat={:.2f} mot={:.2f} raw={:.2f}",
		row.pts, row.mean, row.fLive, row.fSkin, row.fFlat, row.mot, row.raw));
	m_row.push_back(row);
}

void Degrain::PushFrame(const FFrame& src) noexcept {
	if (!src || src.Width() < 32 || src.Height() < 32)
		return;
	Slot sl;
	sl.pic = std::make_unique<FFrame>();
	sl.pic->Format(FFrame::FormatYUV420P());
	if (!src.ScaleTo(*sl.pic, src.Width(), src.Height(),
			FFrame::Resample::Default, FFrame::Scaler::Sws)) {
		Log(Level::Warning, "measure: ScaleTo YUV420P failed; sample dropped");
		return;
	}
	sl.pts = src.Pts();
	uint64_t acc = 0;
	int n = 0;
	const FFrame& g = *sl.pic;
	if (g.Data(0)) {
		const int stepX = std::max(2, g.Width() / 64);
		const int stepY = std::max(2, g.Height() / 64);
		for (int y = 0; y < g.Height(); y += stepY) {
			const uint8_t* row = g.Data(0) + y * g.Linesize(0);
			for (int x = 0; x < g.Width(); x += stepX) {
				acc += row[x];
				++n;
			}
		}
	}
	sl.mean = n ? static_cast<double>(acc) / static_cast<double>(n) : 0.0;
	m_ring.push_back(std::move(sl));
	++m_frames;
	if (m_ring.size() > static_cast<std::size_t>(RingSpan))
		ScoreCenter(m_ring.size() - 1 - static_cast<std::size_t>(RingSpan));
	while (m_ring.size() > static_cast<std::size_t>(RingMax)) {
		ScoreCenter(0);
		m_ring.pop_front();
	}
}

void Degrain::FlushRing() noexcept {
	while (!m_ring.empty()) {
		ScoreCenter(0);
		m_ring.pop_front();
	}
}

void Degrain::Decide() noexcept {
	FlushRing();
	m_voted = true;
	m_seg.clear();
	m_ran = 0;
	m_skipped = 0;
	if (m_row.empty()) {
		Log(Level::Warning, "no samples; leaving video untouched");
		return;
	}

	for (std::size_t i = 0; i < m_row.size(); ++i) {
		const int a = static_cast<int>(i) - FlashWin;
		const int b = static_cast<int>(i) + FlashWin;
		std::vector<double> w;
		for (int j = std::max(0, a); j <= std::min(static_cast<int>(m_row.size()) - 1, b); ++j)
			w.push_back(m_row[static_cast<std::size_t>(j)].mean);
		std::sort(w.begin(), w.end());
		m_row[i].flash = std::fabs(m_row[i].mean - w[w.size() / 2]) >= FlashDelta;
	}

	auto pushSeg = [&](std::size_t from, std::size_t to) noexcept {
		Segment s;
		s.firstPts = m_row[from].pts;
		s.lastPts = m_row[to].pts;
		s.skip = false;
		s.sigma = 0.0;
		std::vector<double> raws, lives, skins, flats, mots, means;
		for (std::size_t i = from; i <= to; ++i) {
			if (m_row[i].flash)
				continue;
			raws.push_back(m_row[i].raw);
			lives.push_back(m_row[i].fLive);
			skins.push_back(m_row[i].fSkin);
			flats.push_back(m_row[i].fFlat);
			mots.push_back(m_row[i].mot);
			means.push_back(m_row[i].mean);
		}
		if (raws.empty()) {
			s.skip = true;
			m_seg.push_back(s);
			++m_skipped;
			return;
		}
		auto medv = [](std::vector<double> v) noexcept {
			std::sort(v.begin(), v.end());
			return v[v.size() / 2];
		};
		const double lv = medv(lives);
		const double sk = medv(skins);
		const double f = medv(flats);
		const double mo = medv(mots);
		const double r = medv(raws);
		const double meanL = medv(means);
		const double cap = std::clamp(m_capIn.value_or(SigmaWall), 0.0, 100.0);
		const bool night = meanL <= NightMean;
		const bool alive = lv >= LiveNeed || sk >= SkinNeed;
		double sig = SigmaFloor;
		if (!night && r < CleanSigma)
			s.skip = true;
		else if (mo >= MotBusy)
			sig = SigmaClose;
		else if (alive && f < FlatNeed)
			sig = SigmaClose;
		else if (!alive && f >= FlatNeed)
			sig = std::min(cap, std::max(r, SigmaFloor));
		else if (alive && f >= FlatNeed)
			sig = SigmaMix;
		else
			s.skip = true;
		if (night)
			sig = std::max(sig, SigmaNight);
		if (s.skip) {
			++m_skipped;
		} else {
			s.sigma = std::min(cap, sig);
			++m_ran;
		}
		Log(Level::Debug, std::format(
			"decide pts={}..{} live={:.2f} skin={:.2f} flat={:.2f} mot={:.2f} mean={:.1f} raw={:.2f} sigma={:.2f} skip={}",
			s.firstPts, s.lastPts, lv, sk, f, mo, meanL, r, s.sigma, s.skip));
		m_seg.push_back(s);
	};

	std::size_t start = 0;
	double anchor = m_row[0].mean;
	double rawAnchor = m_row[0].raw;
	int persist = 0;
	for (std::size_t i = 1; i < m_row.size(); ++i) {
		if (m_row[i].flash) {
			persist = 0;
			continue;
		}
		if (std::fabs(m_row[i].mean - anchor) >= CutMean
			|| std::fabs(m_row[i].raw - rawAnchor) >= CutRaw)
			++persist;
		else {
			persist = 0;
			anchor = 0.8 * anchor + 0.2 * m_row[i].mean;
			rawAnchor = 0.8 * rawAnchor + 0.2 * m_row[i].raw;
		}
		if (persist >= CutPersist) {
			if (i > start)
				pushSeg(start, i - 1);
			start = i;
			anchor = m_row[i].mean;
			rawAnchor = m_row[i].raw;
			persist = 0;
		}
	}
	pushSeg(start, m_row.size() - 1);

	std::vector<double> ran;
	for (const auto& s : m_seg)
		if (!s.skip)
			ran.push_back(s.sigma);
	if (ran.empty())
		Log(Level::Warning, "no stable grain stretch; leaving video untouched");
	else {
		std::sort(ran.begin(), ran.end());
		m_sigmaMin = ran.front();
		m_sigmaMax = ran.back();
		m_sigmaP50 = ran[ran.size() / 2];
		Log(Level::Notice, std::format(
			"stretches ran={} skip={} sigma min/p50/max={:.2f}/{:.2f}/{:.2f} frames={}",
			m_ran, m_skipped, m_sigmaMin, m_sigmaP50, m_sigmaMax, m_frames));
	}
	m_row.clear();
	m_ring.clear();
}

const Degrain::Segment* Degrain::Find(int64_t pts) const noexcept {
	for (const auto& s : m_seg)
		if (pts >= s.firstPts && pts <= s.lastPts)
			return &s;
	return nullptr;
}

std::string Degrain::Chain() const noexcept {
	return std::format(
		"fftdnoiz=sigma={:.3f}:prev=1:next=1:block=32:overlap=0.5",
		m_sigma);
}

void Degrain::Measure(const Pipeline::Frame& frame) noexcept {
	if (frame.Type() != Type::Video)
		return;
	const FFrame& src = AVFrame();
	if (!src)
		return;
	PushFrame(src);
}

void Degrain::Process(const Pipeline::Frame& frame) noexcept {
	if (!m_voted)
		Decide();
	if (frame.Type() != Type::Video)
		return;
	const FFrame& src = AVFrame();
	if (!src || src.Width() <= 0 || src.Height() <= 0)
		return;
	if (src.Hardware()) {
		Fail("hardware frame; decode to software first");
		return;
	}
	const Segment* s = Find(src.Pts());
	if (!s || s->skip || s->sigma <= 0.0)
		return;

	if (std::fabs(m_sigma - s->sigma) > 0.049)
		m_graph.reset();
	m_sigma = s->sigma;
	const std::string chain = Chain();
	if (!m_graph) {
		FGraph opened = FGraph::Open(src, chain);
		if (!opened) {
			Fail("AVFilterGraph::Open failed");
			return;
		}
		m_graph = std::make_unique<FGraph>(std::move(opened));
		Log(Level::Debug, std::format("graph sigma={:.3f} pts={}", m_sigma, src.Pts()));
	} else if (!m_graph->Ensure(src, chain)) {
		Fail("AVFilterGraph::Ensure failed");
		return;
	}

	FFrame out;
	if (!m_graph->Filter(src, out)) {
		Fail("AVFilterGraph::Filter failed");
		return;
	}
	if (out.Width() <= 0 || out.Height() <= 0 || out.Format() == FFrame::FormatNone()) {
		Log(Level::Debug, "wait (EAGAIN)");
		return;
	}
	out.ColorRange(src.ColorRange());
	out.ColorSpace(src.ColorSpace());
	out.ColorPrimaries(src.ColorPrimaries());
	out.ColorTransfer(src.ColorTransfer());
	out.ChromaLocation(src.ChromaLocation());
	out.SampleAspectRatio(src.SampleAspectRatio());
	Save(std::move(out));
}

void Degrain::Eof() noexcept {
	if (!m_voted)
		Decide();
	if (!m_graph)
		return;
	for (;;) {
		FFrame out;
		if (!m_graph->Flush(out)) {
			Fail("AVFilterGraph::Flush failed");
			break;
		}
		if (out.Width() <= 0 || out.Height() <= 0 || out.Format() == FFrame::FormatNone())
			break;
		Save(std::move(out));
	}
	m_graph.reset();
}

class StormByte::Multimedia::Pipeline::Filter::Report Degrain::Report() const noexcept {
	const bool ok = m_voted && m_ran > 0;
	std::map<std::string, std::string> data;
	data.emplace("status", ok ? "ok" : "noop");
	data.emplace("frames", std::to_string(m_frames));
	data.emplace("ran", std::to_string(m_ran));
	data.emplace("skipped", std::to_string(m_skipped));
	data.emplace("stretches", std::to_string(m_seg.size()));
	data.emplace("sigma", std::format("{:.3f}", m_sigmaP50));
	data.emplace("sigma_min", std::format("{:.3f}", m_sigmaMin));
	data.emplace("sigma_max", std::format("{:.3f}", m_sigmaMax));
	data.emplace("sigma_p50", std::format("{:.3f}", m_sigmaP50));
	return Filter::Report(ok ? Filter::Report::Status::Ok : Filter::Report::Status::None,
		std::move(data));
}
