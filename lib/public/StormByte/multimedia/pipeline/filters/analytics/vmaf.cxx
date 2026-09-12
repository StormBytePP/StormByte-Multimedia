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

#include <StormByte/multimedia/pipeline/filters/analytics/vmaf.hxx>
#include <StormByte/multimedia/pipeline/item.hxx>
#include <StormByte/multimedia/type.hxx>

#include <algorithm>
#include <cstring>
#include <format>
#include <thread>

extern "C" {
	#include <libavutil/frame.h>
	#include <libavutil/pixdesc.h>
	#include <libavutil/pixfmt.h>
	#include <libswscale/swscale.h>
	#include <libvmaf/libvmaf.h>
	#include <libvmaf/model.h>
	#include <libvmaf/picture.h>
}

using StormByte::Multimedia::Pipeline::Filter::Video::VMAF;
using StormByte::Multimedia::Pipeline::Producer;
using StormByte::Multimedia::Type;
using StormByte::Logger::Level;

namespace {
	enum VmafPixelFormat Pix(const ::AVFrame* raw) noexcept {
		if (!raw)
			return VMAF_PIX_FMT_UNKNOWN;
		switch (raw->format) {
			case AV_PIX_FMT_YUV420P:
			case AV_PIX_FMT_YUV420P10LE:
			case AV_PIX_FMT_YUV420P12LE:
				return VMAF_PIX_FMT_YUV420P;
			case AV_PIX_FMT_YUV422P:
			case AV_PIX_FMT_YUV422P10LE:
				return VMAF_PIX_FMT_YUV422P;
			case AV_PIX_FMT_YUV444P:
			case AV_PIX_FMT_YUV444P10LE:
				return VMAF_PIX_FMT_YUV444P;
			case AV_PIX_FMT_GRAY8:
			case AV_PIX_FMT_GRAY10LE:
				return VMAF_PIX_FMT_YUV400P;
			default:
				return VMAF_PIX_FMT_UNKNOWN;
		}
	}

	unsigned Bpc(const ::AVFrame* raw) noexcept {
		if (!raw)
			return 8;
		const AVPixFmtDescriptor* desc = av_pix_fmt_desc_get(static_cast<AVPixelFormat>(raw->format));
		if (!desc || desc->comp[0].depth <= 8)
			return 8;
		if (desc->comp[0].depth <= 10)
			return 10;
		return 12;
	}

	void CopyPlane(const uint8_t* src, int srcStride, uint8_t* dst, ptrdiff_t dstStride,
		int bytes, int height) noexcept {
		for (int y = 0; y < height; ++y)
			std::memcpy(dst + y * dstStride, src + y * srcStride, static_cast<std::size_t>(bytes));
	}
}

/*
* Analytics leaf. Route::Add only accepts Process / Packet / Analytics.
*
* Construction names the node ("vmaf") so logs and Report dumps
* can tell filters apart. Do not Launch() here: Route::Add does.
*
* Two looks arrive on the same Process: Producer::Decoder is the
* reference, Producer::Encoder is the distorted reconstruct.
* Clone the raw AVFrame; Work will Drain the StormByte Frame
* after this returns.
*/
VMAF::VMAF(std::shared_ptr<StormByte::Logger::Log> log, std::string model) noexcept
: Filter::Analytics(std::move(log), "vmaf"),
	m_modelName(std::move(model)),
	m_vmaf(nullptr), m_model(nullptr),
	m_width(0), m_height(0),
	m_index(0), m_scored(0), m_failed(false) {}

VMAF::~VMAF() noexcept {
	Clean();
}

enum Type VMAF::Media() const noexcept {
	return Type::Video;
}

void VMAF::DropParked() noexcept {
	for (auto* raw : m_ref)
		av_frame_free(&raw);
	for (auto* raw : m_dist)
		av_frame_free(&raw);
	m_ref.clear();
	m_dist.clear();
}

void VMAF::Clean() noexcept {
	DropParked();
	if (m_vmaf) {
		vmaf_close(m_vmaf);
		m_vmaf = nullptr;
	}
	if (m_model) {
		vmaf_model_destroy(m_model);
		m_model = nullptr;
	}
	m_width = 0;
	m_height = 0;
	m_index = 0;
	m_scored = 0;
	m_mean.reset();
	m_min.reset();
	m_failed = false;
}

void VMAF::Setup() noexcept {
	Clean();
	Log(Level::Debug, std::format("{} setup model={}", Name(), m_modelName));
	VmafConfiguration cfg{};
	cfg.log_level = VMAF_LOG_LEVEL_WARNING;
	cfg.n_threads = std::max(1u, std::thread::hardware_concurrency());
	cfg.n_subsample = 1;
	if (vmaf_init(&m_vmaf, cfg) != 0) {
		Log(Level::Warning, Name() + " vmaf_init failed");
		m_failed = true;
		return;
	}
	VmafModelConfig modelCfg{};
	modelCfg.name = "vmaf";
	modelCfg.flags = VMAF_MODEL_FLAGS_DEFAULT;
	if (vmaf_model_load(&m_model, &modelCfg, m_modelName.c_str()) != 0) {
		Log(Level::Warning, std::format("{} vmaf_model_load({}) failed", Name(), m_modelName));
		m_failed = true;
		return;
	}
	if (vmaf_use_features_from_model(m_vmaf, m_model) != 0) {
		Log(Level::Warning, Name() + " vmaf_use_features_from_model failed");
		m_failed = true;
		return;
	}
	Log(Level::Debug, std::format("{} libvmaf ready threads={}", Name(), cfg.n_threads));
}

bool VMAF::Fill(const ::AVFrame* raw, int tw, int th, void* out) noexcept {
	auto* pic = static_cast<VmafPicture*>(out);
	if (!raw || !pic || tw <= 0 || th <= 0)
		return false;
	const unsigned bpc = Bpc(raw);
	const auto fmt = Pix(raw);
	if (fmt == VMAF_PIX_FMT_UNKNOWN)
		return false;
	if (vmaf_picture_alloc(pic, fmt, bpc, static_cast<unsigned>(tw), static_cast<unsigned>(th)) != 0)
		return false;

	const bool scale = raw->width != tw || raw->height != th;
	const ::AVFrame* src = raw;
	::AVFrame* scaled = nullptr;
	if (scale) {
		scaled = av_frame_alloc();
		if (!scaled) {
			vmaf_picture_unref(pic);
			return false;
		}
		scaled->format = raw->format;
		scaled->width = tw;
		scaled->height = th;
		if (av_frame_get_buffer(scaled, 32) < 0) {
			av_frame_free(&scaled);
			vmaf_picture_unref(pic);
			return false;
		}
		SwsContext* sws = sws_getContext(
			raw->width, raw->height, static_cast<AVPixelFormat>(raw->format),
			tw, th, static_cast<AVPixelFormat>(raw->format),
			SWS_BICUBIC, nullptr, nullptr, nullptr);
		if (!sws) {
			av_frame_free(&scaled);
			vmaf_picture_unref(pic);
			return false;
		}
		sws_scale(sws, raw->data, raw->linesize, 0, raw->height, scaled->data, scaled->linesize);
		sws_freeContext(sws);
		src = scaled;
	}

	const int planes = (fmt == VMAF_PIX_FMT_YUV400P) ? 1 : 3;
	const int bytesPel = (bpc > 8) ? 2 : 1;
	for (int p = 0; p < planes; ++p) {
		if (!src->data[p] || !pic->data[p])
			continue;
		const int height = static_cast<int>(pic->h[p]);
		const int widthBytes = static_cast<int>(pic->w[p]) * bytesPel;
		CopyPlane(src->data[p], src->linesize[p],
			static_cast<uint8_t*>(pic->data[p]), pic->stride[p],
			widthBytes, height);
	}
	av_frame_free(&scaled);
	return true;
}

void VMAF::Score(const ::AVFrame* ref, const ::AVFrame* dist, unsigned index) noexcept {
	if (!m_vmaf || !ref || !dist)
		return;
	if (ref->width <= 0 || ref->height <= 0 || dist->width <= 0 || dist->height <= 0) {
		Log(Level::Warning, Name() + " look has no picture size, skip pair");
		return;
	}
	if (m_width == 0) {
		m_width = ref->width;
		m_height = ref->height;
		if (dist->width != m_width || dist->height != m_height)
			Log(Level::Notice, std::format("{} geometry ref={}x{} dist={}x{}, scaling dist",
				Name(), m_width, m_height, dist->width, dist->height));
	}
	else if (ref->width != m_width || ref->height != m_height) {
		Log(Level::Warning, std::format("{} skip pair, ref size {}x{} latch {}x{}",
			Name(), ref->width, ref->height, m_width, m_height));
		return;
	}
	VmafPicture pref{};
	VmafPicture pdist{};
	if (!Fill(ref, m_width, m_height, &pref)
		|| !Fill(dist, m_width, m_height, &pdist)) {
		Log(Level::Warning, std::format(
			"{} skip pair, fill failed ref={}x{} dist={}x{} latch={}x{}",
			Name(), ref->width, ref->height, dist->width, dist->height,
			m_width, m_height));
		vmaf_picture_unref(&pref);
		vmaf_picture_unref(&pdist);
		return;
	}
	if (vmaf_read_pictures(m_vmaf, &pref, &pdist, index) != 0) {
		Log(Level::Warning, Name() + " skip pair, vmaf_read_pictures failed");
		return;
	}
	++m_scored;
	if (Sparse(0))
		Log(Level::LowLevel, std::format("{} scored index={} n={}", Name(), index, m_scored));
	MaybeThrottle(0);
}

/*
* Index only advances when libvmaf accepted the pair. A gap
* makes vmaf_score_pooled walk empty slots and fail the Report
* after hundreds of good scores.
*/
void VMAF::Drain() noexcept {
	while (!m_ref.empty() && !m_dist.empty()) {
		::AVFrame* ref = m_ref.front();
		::AVFrame* dist = m_dist.front();
		m_ref.pop_front();
		m_dist.pop_front();
		const unsigned before = m_scored;
		Score(ref, dist, m_index);
		if (m_scored > before)
			++m_index;
		av_frame_free(&ref);
		av_frame_free(&dist);
	}
}

void VMAF::Process(const Pipeline::Frame& frame) noexcept {
	if (m_failed)
		return;
	if (frame.Type() != Type::Video)
		return;
	const ::AVFrame* raw = AVFrame();
	if (!raw) {
		Log(Level::Warning, std::format("{} frame has no backend producer={}",
			Name(), static_cast<int>(frame.Producer())));
		return;
	}
	if (frame.Producer() != Producer::Decoder
		&& frame.Producer() != Producer::Encoder)
		return;
	::AVFrame* clone = av_frame_clone(raw);
	if (!clone) {
		Log(Level::Warning, Name() + " av_frame_clone failed");
		return;
	}
	if (frame.Producer() == Producer::Decoder)
		m_ref.push_back(clone);
	else
		m_dist.push_back(clone);
	if (Sparse(frame.Track()))
		Log(Level::LowLevel, std::format("{} park producer={} ref={} dist={}",
			Name(), static_cast<int>(frame.Producer()), m_ref.size(), m_dist.size()));
	MaybeThrottle(frame.Track());
	Drain();
}

void VMAF::Eof() noexcept {
	Drain();
	Log(Level::Debug, std::format("{} eof scored={} ref={} dist={} failed={}",
		Name(), m_scored, m_ref.size(), m_dist.size(), m_failed));
	if (!m_ref.empty() || !m_dist.empty())
		Log(Level::Warning, std::format("{} leftover looks ref={} dist={}",
			Name(), m_ref.size(), m_dist.size()));
	DropParked();
	if (m_vmaf && m_scored > 0)
		vmaf_read_pictures(m_vmaf, nullptr, nullptr, 0);
	if (m_failed || !m_vmaf || !m_model || m_scored == 0) {
		if (!m_failed)
			Log(Level::Warning, std::format("{} no scored pairs", Name()));
		m_failed = true;
		return;
	}
	double mean = 0;
	double mn = 0;
	const unsigned last = m_scored - 1;
	if (vmaf_score_pooled(m_vmaf, m_model, VMAF_POOL_METHOD_MEAN, &mean, 0, last) != 0
		|| vmaf_score_pooled(m_vmaf, m_model, VMAF_POOL_METHOD_MIN, &mn, 0, last) != 0) {
		Log(Level::Warning, Name() + " vmaf_score_pooled failed");
		m_failed = true;
		return;
	}
	m_mean = mean;
	m_min = mn;
	Log(Level::Notice, std::format("{} mean={:.3f} min={:.3f} n={} latch={}x{}",
		Name(), mean, mn, m_scored, m_width, m_height));
}

class StormByte::Multimedia::Pipeline::Filter::Report VMAF::Report() const noexcept {
	if (m_failed || !m_mean)
		return {Filter::Report::Status::Failed, {
			{"model", m_modelName},
			{"scored", std::to_string(m_scored)},
			{"width", std::to_string(m_width)},
			{"height", std::to_string(m_height)}
		}};
	return {Filter::Report::Status::Ok, {
		{"model", m_modelName},
		{"vmaf_mean", std::format("{:.6f}", *m_mean)},
		{"vmaf_min", std::format("{:.6f}", *m_min)},
		{"frames", std::to_string(m_scored)},
		{"width", std::to_string(m_width)},
		{"height", std::to_string(m_height)}
	}};
}
