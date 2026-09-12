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

#include <StormByte/multimedia/pipeline/filters/video/watermark.hxx>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <format>
#include <fstream>
#include <utility>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/frame.h>
	#include <libavutil/pixdesc.h>
	#include <libavutil/pixfmt.h>
	#include <libswscale/swscale.h>
}

using namespace StormByte::Multimedia::Pipeline::Filter::Video;

namespace {
	const ::AVCodec* CodecFromPath(const std::filesystem::path& path) noexcept {
		std::string ext = path.extension().string();
		for (char& c : ext)
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		if (ext == ".png")
			return avcodec_find_decoder(AV_CODEC_ID_PNG);
		if (ext == ".jpg" || ext == ".jpeg")
			return avcodec_find_decoder(AV_CODEC_ID_MJPEG);
		if (ext == ".webp")
			return avcodec_find_decoder(AV_CODEC_ID_WEBP);
		if (ext == ".bmp")
			return avcodec_find_decoder(AV_CODEC_ID_BMP);
		return avcodec_find_decoder(AV_CODEC_ID_MJPEG);
	}

	int SampleY8(const ::AVFrame* src, int x, int y) noexcept {
		if (!src->data[0] || x < 0 || y < 0 || x >= src->width || y >= src->height)
			return 0;
		return src->data[0][y * src->linesize[0] + x];
	}

	int RowMeanY8(const ::AVFrame* src, int row) noexcept {
		unsigned sum = 0;
		const int step = src->width > 64 ? src->width / 64 : 1;
		int n = 0;
		for (int x = 0; x < src->width; x += step) {
			sum += static_cast<unsigned>(SampleY8(src, x, row));
			++n;
		}
		return n ? static_cast<int>(sum / static_cast<unsigned>(n)) : 0;
	}

	int ColMeanY8(const ::AVFrame* src, int col) noexcept {
		unsigned sum = 0;
		const int step = src->height > 64 ? src->height / 64 : 1;
		int n = 0;
		for (int y = 0; y < src->height; y += step) {
			sum += static_cast<unsigned>(SampleY8(src, col, y));
			++n;
		}
		return n ? static_cast<int>(sum / static_cast<unsigned>(n)) : 0;
	}

	int ScanBar(int limit, int cap, const auto& meanAt) noexcept {
		int bar = 0;
		int noise = 0;
		while (bar < limit) {
			const int mean = meanAt(bar);
			if (mean <= cap) {
				noise = 0;
				++bar;
				continue;
			}
			if (mean <= cap + 8 && noise < 2) {
				++noise;
				++bar;
				continue;
			}
			break;
		}
		return bar - noise;
	}

	std::pair<int, int> PairBars(int a, int b) noexcept {
		const int lo = std::min(a, b);
		const int hi = std::max(a, b);
		if (lo < 4)
			return {0, 0};
		if (hi > 0 && lo * 3 < hi)
			return {lo, lo};
		return {a, b};
	}

	void ApplyLut(::AVFrame* gray, const std::array<uint8_t, 256>& lut) noexcept {
		if (!gray || !gray->data[0])
			return;
		for (int y = 0; y < gray->height; ++y) {
			uint8_t* row = gray->data[0] + y * gray->linesize[0];
			for (int x = 0; x < gray->width; ++x)
				row[x] = lut[row[x]];
		}
	}

	std::array<uint8_t, 256> StretchLut(int lo, int hi) noexcept {
		std::array<uint8_t, 256> lut {};
		if (hi <= lo) {
			for (int i = 0; i < 256; ++i)
				lut[static_cast<std::size_t>(i)] = static_cast<uint8_t>(i);
			return lut;
		}
		for (int i = 0; i < 256; ++i) {
			if (i <= lo)
				lut[static_cast<std::size_t>(i)] = 0;
			else if (i >= hi)
				lut[static_cast<std::size_t>(i)] = 255;
			else
				lut[static_cast<std::size_t>(i)] = static_cast<uint8_t>(((i - lo) * 255) / (hi - lo));
		}
		return lut;
	}

	std::array<uint8_t, 256> GammaLut(double gamma) noexcept {
		std::array<uint8_t, 256> lut {};
		for (int i = 0; i < 256; ++i)
			lut[static_cast<std::size_t>(i)] = static_cast<uint8_t>(
				std::clamp(std::pow(static_cast<double>(i) / 255.0, gamma) * 255.0, 0.0, 255.0));
		return lut;
	}

	struct Probe {
		int top = 0;
		int bottom = 0;
		int left = 0;
		int right = 0;
	};

	Probe Measure(::AVFrame* gray) noexcept {
		Probe out;
		const int midY = RowMeanY8(gray, gray->height / 2);
		const int midX = ColMeanY8(gray, gray->width / 2);
		const int edgeTop = RowMeanY8(gray, 0);
		const int edgeBot = RowMeanY8(gray, gray->height - 1);
		const int edgeLeft = ColMeanY8(gray, 0);
		const int edgeRight = ColMeanY8(gray, gray->width - 1);
		const int picture = std::max(midY, midX);
		const int maxY = gray->height / 3;
		const int maxX = gray->width / 4;

		const int edgeY = std::min(edgeTop, edgeBot);
		if (picture >= edgeY + 16) {
			const int cap = edgeY + std::max(10, (picture - edgeY) / 4);
			out.top = ScanBar(maxY, cap, [&](int i) { return RowMeanY8(gray, i); });
			out.bottom = ScanBar(maxY, cap,
				[&](int i) { return RowMeanY8(gray, gray->height - 1 - i); });
			const auto pair = PairBars(out.top, out.bottom);
			out.top = pair.first;
			out.bottom = pair.second;
		}

		const int edgeX = std::min(edgeLeft, edgeRight);
		if (picture >= edgeX + 16) {
			const int cap = edgeX + std::max(10, (picture - edgeX) / 4);
			out.left = ScanBar(maxX, cap, [&](int i) { return ColMeanY8(gray, i); });
			out.right = ScanBar(maxX, cap,
				[&](int i) { return ColMeanY8(gray, gray->width - 1 - i); });
			const auto pair = PairBars(out.left, out.right);
			out.left = pair.first;
			out.right = pair.second;
		}
		return out;
	}

	bool Boxed(const Probe& p) noexcept {
		return (p.top > 4 && p.bottom > 4) || (p.left > 4 && p.right > 4);
	}

	std::pair<int, int> Place(int logoW, int logoH,
		const std::optional<Anchor>& anchor,
		const std::optional<StormByte::Multimedia::Property::Point>& point,
		int margin, int x0, int y0, int aw, int ah) noexcept {
		if (point.has_value())
			return {point->X(), point->Y()};
		const Anchor a = anchor.value_or(Anchor::BottomRight);
		int x = x0 + margin;
		int y = y0 + margin;
		switch (a) {
			case Anchor::TopLeft:		x = x0 + margin; y = y0 + margin; break;
			case Anchor::TopCenter:		x = x0 + (aw - logoW) / 2; y = y0 + margin; break;
			case Anchor::TopRight:		x = x0 + aw - logoW - margin; y = y0 + margin; break;
			case Anchor::CenterLeft:	x = x0 + margin; y = y0 + (ah - logoH) / 2; break;
			case Anchor::Center:		x = x0 + (aw - logoW) / 2; y = y0 + (ah - logoH) / 2; break;
			case Anchor::CenterRight:	x = x0 + aw - logoW - margin; y = y0 + (ah - logoH) / 2; break;
			case Anchor::BottomLeft:	x = x0 + margin; y = y0 + ah - logoH - margin; break;
			case Anchor::BottomCenter:	x = x0 + (aw - logoW) / 2; y = y0 + ah - logoH - margin; break;
			case Anchor::BottomRight:	x = x0 + aw - logoW - margin; y = y0 + ah - logoH - margin; break;
		}
		return {x, y};
	}

	void Blend(uint8_t* dst, int dstLinesize, int frameW, int frameH,
		const uint8_t* logo, int logoW, int logoH, int x, int y, unsigned opacity) noexcept {
		const int alphaScale = static_cast<int>(opacity);
		for (int row = 0; row < logoH; ++row) {
			const int fy = y + row;
			if (fy < 0 || fy >= frameH)
				continue;
			uint8_t* line = dst + fy * dstLinesize;
			const uint8_t* src = logo + row * logoW * 4;
			for (int col = 0; col < logoW; ++col) {
				const int fx = x + col;
				if (fx < 0 || fx >= frameW)
					continue;
				const uint8_t* px = src + col * 4;
				const int a = (static_cast<int>(px[3]) * alphaScale) / 100;
				if (a <= 0)
					continue;
				uint8_t* d = line + fx * 4;
				const int ia = 255 - a;
				d[0] = static_cast<uint8_t>((d[0] * ia + px[0] * a) / 255);
				d[1] = static_cast<uint8_t>((d[1] * ia + px[1] * a) / 255);
				d[2] = static_cast<uint8_t>((d[2] * ia + px[2] * a) / 255);
				d[3] = 255;
			}
		}
	}
}

Watermark::Watermark(std::shared_ptr<StormByte::Logger::Log> log,
	const std::filesystem::path& logo, Anchor anchor,
	unsigned opacity, int margin) noexcept
: Filter::Process(std::move(log), "watermark"), m_path(logo), m_anchor(anchor),
	m_opacity(std::min(opacity, 100u)), m_margin(margin),
	m_logoWidth(0), m_logoHeight(0), m_loaded(false), m_decoded(false),
	m_released(false), m_barTop(0), m_barBottom(0), m_barLeft(0), m_barRight(0),
	m_stable(0), m_lumaW(0), m_lumaH(0), m_lumaFmt(AV_PIX_FMT_NONE),
	m_swsLuma(nullptr), m_luma(nullptr) {}

Watermark::Watermark(std::shared_ptr<StormByte::Logger::Log> log,
	const std::filesystem::path& logo,
	StormByte::Multimedia::Property::Point position, unsigned opacity) noexcept
: Filter::Process(std::move(log), "watermark"), m_path(logo), m_point(position),
	m_opacity(std::min(opacity, 100u)), m_margin(0),
	m_logoWidth(0), m_logoHeight(0), m_loaded(false), m_decoded(false),
	m_released(false), m_barTop(0), m_barBottom(0), m_barLeft(0), m_barRight(0),
	m_stable(0), m_lumaW(0), m_lumaH(0), m_lumaFmt(AV_PIX_FMT_NONE),
	m_swsLuma(nullptr), m_luma(nullptr) {}

Watermark::~Watermark() noexcept {
	DropScale();
}

enum StormByte::Multimedia::Type Watermark::Media() const noexcept {
	return StormByte::Multimedia::Type::Video;
}

void Watermark::DropScale() noexcept {
	if (m_swsLuma) {
		sws_freeContext(static_cast<::SwsContext*>(m_swsLuma));
		m_swsLuma = nullptr;
	}
	if (m_luma) {
		av_frame_free(&m_luma);
		m_luma = nullptr;
	}
	m_lumaW = 0;
	m_lumaH = 0;
	m_lumaFmt = AV_PIX_FMT_NONE;
}

void Watermark::Clean() noexcept {
	m_bytes.clear();
	m_rgba.clear();
	m_logoWidth = 0;
	m_logoHeight = 0;
	m_loaded = false;
	m_decoded = false;
	m_released = false;
	m_barTop = 0;
	m_barBottom = 0;
	m_barLeft = 0;
	m_barRight = 0;
	m_stable = 0;
	DropScale();
}

void Watermark::DisableLogo(std::string_view why) noexcept {
	Log(StormByte::Logger::Level::Warning,
		std::format("Video/watermark disabled: {}", why));
	m_opacity = 0;
	m_bytes.clear();
	m_rgba.clear();
	m_logoWidth = 0;
	m_logoHeight = 0;
}

void Watermark::Setup() noexcept {
	if (m_opacity == 0)
		return;
	(void)LoadFile();
}

bool Watermark::LoadFile() noexcept {
	if (m_loaded)
		return !m_bytes.empty();
	m_loaded = true;

	std::ifstream in(m_path, std::ios::binary);
	if (!in) {
		DisableLogo("cannot open " + m_path.string());
		return false;
	}
	in.seekg(0, std::ios::end);
	const auto size = in.tellg();
	if (size <= 0) {
		DisableLogo("empty logo");
		return false;
	}
	in.seekg(0, std::ios::beg);
	m_bytes.resize(static_cast<std::size_t>(size));
	in.read(reinterpret_cast<char*>(m_bytes.data()), size);
	if (!in) {
		m_bytes.clear();
		DisableLogo("failed to read " + m_path.string());
		return false;
	}
	return true;
}

bool Watermark::DecodeLogo() noexcept {
	if (m_decoded)
		return !m_rgba.empty();
	m_decoded = true;
	if (!LoadFile())
		return false;

	const ::AVCodec* codec = CodecFromPath(m_path);
	if (!codec) {
		DisableLogo("no decoder for logo");
		return false;
	}

	::AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx || avcodec_open2(ctx, codec, nullptr) < 0) {
		avcodec_free_context(&ctx);
		DisableLogo("failed to open logo decoder");
		return false;
	}

	::AVPacket* pkt = av_packet_alloc();
	::AVFrame* decoded = av_frame_alloc();
	if (!pkt || !decoded) {
		av_packet_free(&pkt);
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		DisableLogo("out of memory decoding logo");
		return false;
	}

	pkt->data = reinterpret_cast<uint8_t*>(m_bytes.data());
	pkt->size = static_cast<int>(m_bytes.size());
	const bool ok = avcodec_send_packet(ctx, pkt) >= 0 && avcodec_receive_frame(ctx, decoded) >= 0;
	pkt->data = nullptr;
	pkt->size = 0;
	av_packet_free(&pkt);

	if (!ok || decoded->width <= 0 || decoded->height <= 0) {
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		DisableLogo("failed to decode logo");
		return false;
	}

	::AVFrame* rgba = av_frame_alloc();
	if (!rgba) {
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		DisableLogo("out of memory decoding logo");
		return false;
	}
	rgba->format = AV_PIX_FMT_RGBA;
	rgba->width = decoded->width;
	rgba->height = decoded->height;
	if (av_frame_get_buffer(rgba, 0) < 0) {
		av_frame_free(&rgba);
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		DisableLogo("failed to allocate RGBA logo");
		return false;
	}

	::SwsContext* sws = sws_getContext(
		decoded->width, decoded->height, static_cast<::AVPixelFormat>(decoded->format),
		rgba->width, rgba->height, AV_PIX_FMT_RGBA,
		SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!sws || sws_scale(sws, decoded->data, decoded->linesize, 0, decoded->height,
			rgba->data, rgba->linesize) <= 0) {
		if (sws)
			sws_freeContext(sws);
		av_frame_free(&rgba);
		av_frame_free(&decoded);
		avcodec_free_context(&ctx);
		DisableLogo("failed to convert logo to RGBA");
		return false;
	}
	sws_freeContext(sws);

	m_logoWidth = rgba->width;
	m_logoHeight = rgba->height;
	m_rgba.resize(static_cast<std::size_t>(m_logoWidth) * static_cast<std::size_t>(m_logoHeight) * 4);
	for (int row = 0; row < m_logoHeight; ++row) {
		const uint8_t* src = rgba->data[0] + row * rgba->linesize[0];
		uint8_t* dst = reinterpret_cast<uint8_t*>(m_rgba.data()) +
			static_cast<std::size_t>(row) * static_cast<std::size_t>(m_logoWidth) * 4;
		std::copy(src, src + m_logoWidth * 4, dst);
	}

	av_frame_free(&rgba);
	av_frame_free(&decoded);
	avcodec_free_context(&ctx);
	return true;
}

::AVFrame* Watermark::Luma(::AVFrame* src) noexcept {
	if (!src || src->width <= 0 || src->height <= 0)
		return nullptr;
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(static_cast<::AVPixelFormat>(src->format));
	if (!desc || (desc->flags & AV_PIX_FMT_FLAG_HWACCEL)) {
		Fail("watermark needs a software frame");
		return nullptr;
	}

	if (m_luma && (m_lumaW != src->width || m_lumaH != src->height || m_lumaFmt != src->format))
		DropScale();

	if (!m_luma) {
		m_luma = av_frame_alloc();
		if (!m_luma) {
			Fail("out of memory");
			return nullptr;
		}
		m_luma->format = AV_PIX_FMT_GRAY8;
		m_luma->width = src->width;
		m_luma->height = src->height;
		if (av_frame_get_buffer(m_luma, 0) < 0) {
			DropScale();
			Fail("failed to allocate luma probe");
			return nullptr;
		}
		m_swsLuma = sws_getContext(
			src->width, src->height, static_cast<::AVPixelFormat>(src->format),
			m_luma->width, m_luma->height, AV_PIX_FMT_GRAY8,
			SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
		if (!m_swsLuma) {
			DropScale();
			Fail("failed to convert frame to luma");
			return nullptr;
		}
		m_lumaW = src->width;
		m_lumaH = src->height;
		m_lumaFmt = src->format;
	}

	if (sws_scale(static_cast<::SwsContext*>(m_swsLuma), src->data, src->linesize,
			0, src->height, m_luma->data, m_luma->linesize) <= 0) {
		Fail("failed to convert frame to luma");
		return nullptr;
	}
	return m_luma;
}

bool Watermark::ProbeBars(::AVFrame* src) noexcept {
	::AVFrame* gray = Luma(src);
	if (!gray || gray->width < 16 || gray->height < 16)
		return false;

	Probe found = Measure(gray);

	if (!Boxed(found)) {
		const int edge = std::min({
			RowMeanY8(gray, 0),
			RowMeanY8(gray, gray->height - 1),
			ColMeanY8(gray, 0),
			ColMeanY8(gray, gray->width - 1)
		});
		const int core = std::max(
			RowMeanY8(gray, gray->height / 2),
			ColMeanY8(gray, gray->width / 2));
		if (core > edge) {
			ApplyLut(gray, StretchLut(edge, core));
			found = Measure(gray);
		}
	}

	if (!Boxed(found)) {
		ApplyLut(gray, GammaLut(0.45));
		found = Measure(gray);
	}

	if (!Boxed(found))
		return false;

	const bool same = found.top <= m_barTop && found.bottom <= m_barBottom
		&& found.left <= m_barLeft && found.right <= m_barRight;

	if (found.top > 0 && found.bottom > 0) {
		if (m_barTop == 0) {
			m_barTop = found.top;
			m_barBottom = found.bottom;
		}
		else {
			m_barTop = std::min(m_barTop, found.top);
			m_barBottom = std::min(m_barBottom, found.bottom);
		}
	}
	if (found.left > 0 && found.right > 0) {
		if (m_barLeft == 0) {
			m_barLeft = found.left;
			m_barRight = found.right;
		}
		else {
			m_barLeft = std::min(m_barLeft, found.left);
			m_barRight = std::min(m_barRight, found.right);
		}
	}

	if (same)
		++m_stable;
	else
		m_stable = 0;
	return true;
}

void Watermark::Process(const Pipeline::Frame&) noexcept {
	if (m_opacity == 0)
		return;

	::AVFrame* src = AVFrame();

	if (m_anchor && !m_point && !m_released) {
		if (!Held())
			Hold(ProbeMax);
		const bool usable = ProbeBars(src);
		const bool boxed = (m_barTop > 4 && m_barBottom > 4)
			|| (m_barLeft > 4 && m_barRight > 4);
		if (usable && boxed && m_stable >= 8) {
			m_released = true;
			Release();
			return;
		}
		if (Held())
			return;
		m_released = true;
	}

	Paint();
}

void Watermark::LastChance(const Pipeline::Frame&) noexcept {
	m_released = true;
	Release();
}

void Watermark::Paint() noexcept {
	if (m_opacity == 0)
		return;
	if (!DecodeLogo())
		return;

	::AVFrame* src = AVFrame();
	if (!src || src->width <= 0 || src->height <= 0) {
		Fail("missing video buffer");
		return;
	}
	const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(static_cast<::AVPixelFormat>(src->format));
	if (!desc || (desc->flags & AV_PIX_FMT_FLAG_HWACCEL)) {
		Fail("watermark needs a software frame");
		return;
	}

	const int x0 = m_point ? 0 : m_barLeft;
	const int y0 = m_point ? 0 : m_barTop;
	const int aw = m_point ? src->width : src->width - m_barLeft - m_barRight;
	const int ah = m_point ? src->height : src->height - m_barTop - m_barBottom;
	if (aw <= 0 || ah <= 0) {
		DisableLogo("active picture is empty");
		return;
	}

	const auto [x, y] = Place(m_logoWidth, m_logoHeight, m_anchor, m_point,
		m_margin, x0, y0, aw, ah);
	if (x < x0 || y < y0 || x + m_logoWidth > x0 + aw || y + m_logoHeight > y0 + ah) {
		DisableLogo("logo does not fit in the active picture");
		return;
	}

	::AVFrame* out = av_frame_alloc();
	if (!out) {
		Fail("out of memory");
		return;
	}
	if (av_frame_copy_props(out, src) < 0) {
		av_frame_free(&out);
		Fail("failed to copy frame properties");
		return;
	}
	out->width = src->width;
	out->height = src->height;
	out->format = AV_PIX_FMT_RGBA;
	if (av_frame_get_buffer(out, 0) < 0) {
		av_frame_free(&out);
		Fail("failed to allocate destination");
		return;
	}

	::SwsContext* toRgba = sws_getContext(
		src->width, src->height, static_cast<::AVPixelFormat>(src->format),
		out->width, out->height, AV_PIX_FMT_RGBA,
		SWS_BILINEAR, nullptr, nullptr, nullptr);
	if (!toRgba || sws_scale(toRgba, src->data, src->linesize, 0, src->height,
			out->data, out->linesize) <= 0) {
		if (toRgba)
			sws_freeContext(toRgba);
		av_frame_free(&out);
		Fail("failed to convert frame to RGBA");
		return;
	}
	sws_freeContext(toRgba);

	Blend(out->data[0], out->linesize[0], out->width, out->height,
		reinterpret_cast<const uint8_t*>(m_rgba.data()),
		m_logoWidth, m_logoHeight, x, y, m_opacity);

	if (src->format != AV_PIX_FMT_RGBA) {
		::AVFrame* restored = av_frame_alloc();
		if (!restored) {
			av_frame_free(&out);
			Fail("out of memory");
			return;
		}
		if (av_frame_copy_props(restored, src) < 0) {
			av_frame_free(&restored);
			av_frame_free(&out);
			Fail("failed to copy frame properties");
			return;
		}
		restored->width = src->width;
		restored->height = src->height;
		restored->format = src->format;
		if (av_frame_get_buffer(restored, 0) < 0) {
			av_frame_free(&restored);
			av_frame_free(&out);
			Fail("failed to allocate destination");
			return;
		}
		::SwsContext* fromRgba = sws_getContext(
			out->width, out->height, AV_PIX_FMT_RGBA,
			restored->width, restored->height, static_cast<::AVPixelFormat>(restored->format),
			SWS_BILINEAR, nullptr, nullptr, nullptr);
		if (!fromRgba || sws_scale(fromRgba, out->data, out->linesize, 0, out->height,
				restored->data, restored->linesize) <= 0) {
			if (fromRgba)
				sws_freeContext(fromRgba);
			av_frame_free(&restored);
			av_frame_free(&out);
			Fail("failed to convert frame back");
			return;
		}
		sws_freeContext(fromRgba);
		av_frame_free(&out);
		out = restored;
	}

	Save(out);
}
