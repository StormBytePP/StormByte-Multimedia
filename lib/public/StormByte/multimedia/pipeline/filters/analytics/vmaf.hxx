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
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/filters/report.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <optional>
#include <string>

struct VmafContext;
struct VmafModel;
struct AVFrame;

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Video
 * @brief Video leaves (Process and Analytics).
 *
 * Inherit @ref Filter::Process to rewrite frames, or
 * @ref Filter::Analytics to observe them. Do not inherit
 * @ref Filter::FFmpeg. Attach with
 * @c job.Filter<VMAF>(log, "vmaf_4k_v0.6.1").
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @class VMAF
	 * @brief Full-reference VMAF. Reference Analytics leaf.
	 *
	 * @par What an Analytics leaf is
	 * Analytics is not on the encode path. Filters CloneTo s the
	 * Decoder tap (reference look, cloned before Process) and
	 * a Route-owned dest look (distorted look) onto this
	 * node. @ref Process sees only @ref Pipeline::Frame.
	 * Reference @ref Item::Producer is @ref Producer::Decoder.
	 * Distorted Producer is @ref Producer::Encoder or
	 * @ref Producer::Remuxer. There is no Packet and no
	 * @ref FFmpeg::Save.
	 *
	 * Implement @ref Setup, @ref Process, @ref Eof, @ref Clean,
	 * @ref Report, @ref Media. Filters launches the worker.
	 *
	 * @par Pairing
	 * Presentation FIFOs per @ref Pipeline::Frame::Track, not Serial
	 * or PTS. Serial is tube lineage and does not identify a
	 * reconstructed picture when the encoder has delay. Each look
	 * leaves avcodec in presentation order, so the nth Decoder tap
	 * frame of a track is the same picture as the nth dest-look
	 * frame of that track. One libvmaf context per track. There is
	 * no pooled mean across tracks.
	 *
	 * @par Geometry
	 * Latched on the first valid reference. Distorted looks
	 * of another size are scaled to that latch. A later
	 * reference that changes width/height is skipped with a
	 * Warning. Scale is not @ref Report::Failed.
	 *
	 * @par Memory
	 * @ref InputCeiling sizes the analytics hopper (tap +
	 * look). That is backpressure, not a park cap. Process
	 * clones into per-track parks with no bound: the
	 * park must cover encoder delay. @ref Drain frees a
	 * pair as soon as both heads exist. After a successful
	 * @c vmaf_read_pictures libvmaf owns the VmafPictures.
	 *
	 * @par When to read @ref Report
	 * Muxer closed is not Eof on this node. Wait Filters::Idle
	 * first. A low score is not Fail. @ref Report::Failed
	 * only when a context or model could not be opened, or
	 * no pair was scored. One track keeps flat keys
	 * (`vmaf_mean`). Several tracks prefix with the origin
	 * index (`0.vmaf_mean`, `1.vmaf_mean`).
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::Analytics
	 * @see StormByte::Multimedia::Pipeline::Filters
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC VMAF: public Filter::Analytics {
		public:
			/**
			 * @brief VMAF with a built-in libvmaf model version.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param model libvmaf version key (`vmaf_4k_v0.6.1`).
			 */
			VMAF(std::shared_ptr<StormByte::Logger::Log> log, std::string model) noexcept;

			VMAF(const VMAF& other) = delete;
			VMAF(VMAF&& other) noexcept = delete;

			/**
			 * @brief Destructor. Closes libvmaf and drops parked looks.
			 */
			~VMAF() noexcept override;

			VMAF& operator=(const VMAF& other) = delete;
			VMAF& operator=(VMAF&& other) noexcept = delete;

			/**
			 * @brief Media this filter handles.
			 * @return Video. Other kinds are ignored in Process.
			 */
			enum StormByte::Multimedia::Type Media() const noexcept override;

			/**
			 * @brief Ceiling of the analytics input hopper.
			 * @return Max items in the tap / look hoppers. Never 0.
			 *
			 * Must exceed encoder delay or the first distorted
			 * look cannot arrive. Does not cap @c m_ref / @c m_dist.
			 */
			std::size_t InputCeiling() const noexcept override {
				return Ceiling;
			}

			/**
			 * @brief Pooled VMAF after EoF.
			 * @return Ok with mean/min/frames, or Failed.
			 *
			 * One scored track: `vmaf_mean` / `vmaf_min` / `frames`.
			 * Several: `0.vmaf_mean`, `1.vmaf_mean` (origin index).
			 * No mean of means.
			 */
			class Filter::Report Report() const noexcept override;

		protected:
			/**
			 * @brief Drops parked looks and closes libvmaf.
			 */
			void Clean() noexcept override;

			/**
			 * @brief Names the model. Contexts open on the first look of each track.
			 */
			void Setup() noexcept override;

			/**
			 * @brief Parks one look and scores when both FIFOs of that track have a head.
			 * @param frame Decoder (ref) or Encoder/Remuxer (dist) video frame.
			 *
			 * Lane is @ref Pipeline::Frame::Track. No extra constructor
			 * argument for N videos.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Flushes libvmaf and pools the score.
			 */
			void Eof() noexcept override;

		private:
			struct Lane {
				VmafContext* vmaf = nullptr;
				VmafModel* model = nullptr;
				std::deque<::AVFrame*> ref;
				std::deque<::AVFrame*> dist;
				int width = 0;
				int height = 0;
				unsigned index = 0;
				unsigned scored = 0;
				std::optional<double> mean;
				std::optional<double> min;
				bool failed = false;
			};

			/**
			 * @brief Copies @p raw into a VmafPicture, scaling to @p tw x @p th.
			 * @param raw Source libav frame.
			 * @param tw Target width (latch).
			 * @param th Target height (latch).
			 * @param out VmafPicture to fill.
			 * @return true if @p out is filled. On failure @p out is unref'd.
			 *
			 * On success libvmaf takes the picture in
			 * @c vmaf_read_pictures. Do not unref after that.
			 */
			bool Fill(const ::AVFrame* raw, int tw, int th, void* out) noexcept;

			/**
			 * @brief Opens a libvmaf context and model on @p lane.
			 */
			bool OpenLane(Lane& lane) noexcept;

			/**
			 * @brief Hands one pair to libvmaf at contiguous @p index.
			 * @param lane Track context.
			 * @param ref Decoder tap look.
			 * @param dist Dest look (Encoder or Remuxer).
			 * @param index Next libvmaf picture index (0..n).
			 *
			 * Does not free @p ref / @p dist. @ref Drain does.
			 */
			void Score(Lane& lane, const ::AVFrame* ref, const ::AVFrame* dist, unsigned index) noexcept;

			/**
			 * @brief Scores while both presentation FIFOs of @p lane have a frame.
			 */
			void Drain(Lane& lane) noexcept;

			/**
			 * @brief Frees parked clones of @p lane.
			 */
			void DropParked(Lane& lane) noexcept;

			/**
			 * @brief Closes libvmaf and drops parked looks of every lane.
			 */
			void DropAll() noexcept;

			static constexpr std::size_t Ceiling = 512;	///< Analytics hopper only
			std::string m_modelName;					///< libvmaf built-in version
			std::map<int, Lane> m_lanes;				///< One context per Frame::Track
	};
}
