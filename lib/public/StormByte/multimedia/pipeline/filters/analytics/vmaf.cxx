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
#include <StormByte/multimedia/pipeline/typedefs.hxx>
#include <StormByte/multimedia/type.hxx>

#include <algorithm>
#include <cstring>
#include <format>
#include <map>
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
using StormByte::Multimedia::Pipeline::ToString;
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

	const char* PixName(const ::AVFrame* raw) noexcept {
		if (!raw)
			return "?";
		const char* name = av_get_pix_fmt_name(static_cast<AVPixelFormat>(raw->format));
		return name ? name : "?";
	}

	bool DistLook(Producer producer) noexcept {
		return producer == Producer::Encoder || producer == Producer::Remuxer;
	}

	void CopyPlane(const uint8_t* src, int srcStride, uint8_t* dst, ptrdiff_t dstStride,
		int bytes, int height) noexcept {
		for (int y = 0; y < height; ++y)
			std::memcpy(dst + y * dstStride, src + y * srcStride, static_cast<std::size_t>(bytes));
	}
}

/*
* Analytics leaf. Filters::Add / Between::Add only accept
* Process / Packet / Analytics.
*
* Construction names the node ("vmaf") so logs and Report dumps
* can tell filters apart. Do not Launch() here: Filters does.
*
* Two looks arrive on the same Process: Producer::Decoder is the
* reference, Producer::Encoder or Producer::Remuxer is the distorted
* reconstruct. Clone the raw AVFrame; Work will Drain the StormByte
* Frame after this returns.
*
* Pairing is presentation FIFO per Frame::Track, not Serial and
* not container PTS. One libvmaf context per track.
*
* Debug is the bugreport level: first looks, latch, ignore, paced
* scored count, eof. Per-frame park/score stays at LowLevel.
*/
VMAF::VMAF(std::shared_ptr<StormByte::Logger::Log> log, std::string model,
	std::optional<unsigned short> threads) noexcept
: Filter::Analytics(std::move(log), "vmaf"),
	m_modelName(std::move(model)), m_threads(threads) {}

VMAF::~VMAF() noexcept {
	Clean();
}

enum Type VMAF::Media() const noexcept {
	return Type::Video;
}

void VMAF::DropParked(Lane& lane) noexcept {
	for (auto* raw : lane.ref)
		av_frame_free(&raw);
	for (auto* raw : lane.dist)
		av_frame_free(&raw);
	lane.ref.clear();
	lane.dist.clear();
}

void VMAF::DropAll() noexcept {
	for (auto& [track, lane] : m_lanes) {
		(void)track;
		DropParked(lane);
		if (lane.vmaf) {
			vmaf_close(lane.vmaf);
			lane.vmaf = nullptr;
		}

		if (lane.model) {
			vmaf_model_destroy(lane.model);
			lane.model = nullptr;
		}
	}

	m_lanes.clear();
}

void VMAF::Clean() noexcept {
	DropAll();
}

bool VMAF::OpenLane(Lane& lane) noexcept {
	VmafConfiguration cfg{};
	cfg.log_level = VMAF_LOG_LEVEL_WARNING;
	unsigned threads = 1;
	if (m_threads)
		threads = std::max<unsigned>(1u, *m_threads);
	else if (const unsigned hw = std::thread::hardware_concurrency(); hw > 0)
		threads = hw;
	cfg.n_threads = threads;
	cfg.n_subsample = 1;
	if (vmaf_init(&lane.vmaf, cfg) != 0) {
		Log(Level::Error, Name() + " vmaf_init failed");
		lane.vmaf = nullptr;
		lane.failed = true;
		return false;
	}

	VmafModelConfig modelCfg{};
	modelCfg.name = "vmaf";
	modelCfg.flags = VMAF_MODEL_FLAGS_DEFAULT;
	if (vmaf_model_load(&lane.model, &modelCfg, m_modelName.c_str()) != 0) {
		Log(Level::Error, std::format("{} vmaf_model_load({}) failed", Name(), m_modelName));
		vmaf_close(lane.vmaf);
		lane.vmaf = nullptr;
		lane.failed = true;
		return false;
	}

	if (vmaf_use_features_from_model(lane.vmaf, lane.model) != 0) {
		Log(Level::Error, Name() + " vmaf_use_features_from_model failed");
		vmaf_close(lane.vmaf);
		lane.vmaf = nullptr;
		vmaf_model_destroy(lane.model);
		lane.model = nullptr;
		lane.failed = true;
		return false;
	}

	Log(Level::Debug, std::format("{} libvmaf ready threads={} subsample=1",
		Name(), cfg.n_threads));
	return true;
}

void VMAF::Setup() noexcept {
	Clean();
	Log(Level::Debug, std::format("{} setup model={}", Name(), m_modelName));
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

void VMAF::Score(Lane& lane, const ::AVFrame* ref, const ::AVFrame* dist, unsigned index) noexcept {
	if (!lane.vmaf || !ref || !dist)
		return;
	if (ref->width <= 0 || ref->height <= 0 || dist->width <= 0 || dist->height <= 0) {
		Log(Level::Warning, Name() + " look has no picture size, skip pair");
		return;
	}

	if (lane.width == 0) {
		lane.width = ref->width;
		lane.height = ref->height;
		Log(Level::Debug, std::format(
			"{} first pair index={} latch={}x{} ref_fmt={} ref_bpc={} ref_pts={} dist={}x{} dist_fmt={} dist_bpc={} dist_pts={}",
			Name(), index, lane.width, lane.height,
			PixName(ref), Bpc(ref), ref->pts,
			dist->width, dist->height,
			PixName(dist), Bpc(dist), dist->pts));
		if (dist->width != lane.width || dist->height != lane.height)
			Log(Level::Notice, std::format("{} geometry ref={}x{} dist={}x{}, scaling dist",
				Name(), lane.width, lane.height, dist->width, dist->height));
	}

	else if (ref->width != lane.width || ref->height != lane.height) {
		Log(Level::Warning, std::format("{} skip pair, ref size {}x{} latch {}x{}",
			Name(), ref->width, ref->height, lane.width, lane.height));
		return;
	}

	VmafPicture pref{};
	VmafPicture pdist{};
	if (!Fill(ref, lane.width, lane.height, &pref)
		|| !Fill(dist, lane.width, lane.height, &pdist)) {
		Log(Level::Warning, std::format(
			"{} skip pair, fill failed ref={}x{} dist={}x{} latch={}x{}",
			Name(), ref->width, ref->height, dist->width, dist->height,
			lane.width, lane.height));
		vmaf_picture_unref(&pref);
		vmaf_picture_unref(&pdist);
		return;
	}

	if (vmaf_read_pictures(lane.vmaf, &pref, &pdist, index) != 0) {
		Log(Level::Warning, Name() + " skip pair, vmaf_read_pictures failed");
		vmaf_picture_unref(&pref);
		vmaf_picture_unref(&pdist);
		return;
	}

	++lane.scored;
	Log(Level::LowLevel, std::format("{} scored index={} n={} ref_pts={} dist_pts={}",
		Name(), index, lane.scored, ref->pts, dist->pts));
}

/*
* Index only advances when libvmaf accepted the pair. A gap
* makes vmaf_score_pooled walk empty slots and fail the Report
* after hundreds of good scores.
*/
void VMAF::Drain(Lane& lane) noexcept {
	while (!lane.ref.empty() && !lane.dist.empty()) {
		::AVFrame* ref = lane.ref.front();
		::AVFrame* dist = lane.dist.front();
		lane.ref.pop_front();
		lane.dist.pop_front();
		const unsigned before = lane.scored;
		Score(lane, ref, dist, lane.index);
		if (lane.scored > before)
			++lane.index;
		av_frame_free(&ref);
		av_frame_free(&dist);
	}
}

void VMAF::Process(const Pipeline::Frame& frame) noexcept {
	if (frame.Type() != Type::Video)
		return;
	const Producer producer = frame.Producer();
	if (producer != Producer::Decoder && !DistLook(producer)) {
		Log(Level::Debug, std::format("{} ignore producer={}", Name(), ToString(producer)));
		return;
	}

	const ::AVFrame* raw = AVFrame();
	if (!raw) {
		Log(Level::Warning, std::format("{} frame has no backend producer={}",
			Name(), ToString(producer)));
		return;
	}

	Lane& lane = m_lanes[frame.Track()];
	if (lane.failed)
		return;
	if (!lane.vmaf && !OpenLane(lane))
		return;

	::AVFrame* clone = av_frame_clone(raw);
	if (!clone) {
		Log(Level::Warning, Name() + " av_frame_clone failed");
		return;
	}

	if (producer == Producer::Decoder) {
		if (lane.scored == 0 && lane.ref.empty())
			Log(Level::Debug, std::format("{} first ref t={} {}x{} fmt={} bpc={} pts={}",
				Name(), frame.Track(), raw->width, raw->height, PixName(raw), Bpc(raw), raw->pts));
		lane.ref.push_back(clone);
	}

	else {
		if (lane.scored == 0 && lane.dist.empty())
			Log(Level::Debug, std::format("{} first dist t={} producer={} {}x{} fmt={} bpc={} pts={}",
				Name(), frame.Track(), ToString(producer),
				raw->width, raw->height, PixName(raw), Bpc(raw), raw->pts));
		lane.dist.push_back(clone);
	}

	Log(Level::LowLevel, std::format("{} park t={} producer={} ref={} dist={}",
		Name(), frame.Track(), ToString(producer), lane.ref.size(), lane.dist.size()));
	Drain(lane);
}

void VMAF::Eof() noexcept {
	unsigned scored = 0;
	for (auto& [track, lane] : m_lanes) {
		Drain(lane);
		Log(Level::Debug, std::format("{} eof t={} scored={} ref={} dist={} failed={} latch={}x{}",
			Name(), track, lane.scored, lane.ref.size(), lane.dist.size(), lane.failed,
			lane.width, lane.height));
		if (!lane.ref.empty() || !lane.dist.empty())
			Log(Level::Warning, std::format("{} leftover looks t={} ref={} dist={}",
				Name(), track, lane.ref.size(), lane.dist.size()));
		DropParked(lane);
		if (lane.vmaf && lane.scored > 0)
			vmaf_read_pictures(lane.vmaf, nullptr, nullptr, 0);
		if (lane.failed || !lane.vmaf || !lane.model || lane.scored == 0) {
			if (!lane.failed)
				Log(Level::Error, std::format("{} t={} no scored pairs", Name(), track));
			lane.failed = true;
			continue;
		}

		double mean = 0;
		double mn = 0;
		const unsigned last = lane.scored - 1;
		if (vmaf_score_pooled(lane.vmaf, lane.model, VMAF_POOL_METHOD_MEAN, &mean, 0, last) != 0
			|| vmaf_score_pooled(lane.vmaf, lane.model, VMAF_POOL_METHOD_MIN, &mn, 0, last) != 0) {
			Log(Level::Error, std::format("{} t={} vmaf_score_pooled failed", Name(), track));
			lane.failed = true;
			continue;
		}

		lane.mean = mean;
		lane.min = mn;
		scored += lane.scored;
		Log(Level::Notice, std::format("{} t={} mean={:.3f} min={:.3f} n={} latch={}x{}",
			Name(), track, mean, mn, lane.scored, lane.width, lane.height));
	}

	if (m_lanes.empty())
		Log(Level::Error, std::format("{} no scored pairs", Name()));
	else if (scored > 0)
		Log(Level::Debug, std::format("{} eof tracks={} scored={}", Name(), m_lanes.size(), scored));
}

class StormByte::Multimedia::Pipeline::Filter::Report VMAF::Report() const noexcept {
	std::map<std::string, std::string> data;
	data.emplace("model", m_modelName);
	bool failed = m_lanes.empty();
	unsigned ok = 0;
	for (const auto& [track, lane] : m_lanes) {
		if (lane.failed || !lane.mean)
			failed = true;
		else
			++ok;
	}

	const bool prefix = m_lanes.size() > 1;
	auto key = [prefix](int track, const char* name) {
		if (!prefix)
			return std::string(name);
		return std::to_string(track) + "." + name;
	};

	for (const auto& [track, lane] : m_lanes) {
		if (lane.mean) {
			data.emplace(key(track, "vmaf_mean"), std::format("{:.6f}", *lane.mean));
			data.emplace(key(track, "vmaf_min"), std::format("{:.6f}", *lane.min));
		}

		data.emplace(key(track, "frames"), std::to_string(lane.scored));
		data.emplace(key(track, "width"), std::to_string(lane.width));
		data.emplace(key(track, "height"), std::to_string(lane.height));
	}

	if (failed || ok == 0)
		return {Filter::Report::Status::Failed, std::move(data)};
	return {Filter::Report::Status::Ok, std::move(data)};
}
