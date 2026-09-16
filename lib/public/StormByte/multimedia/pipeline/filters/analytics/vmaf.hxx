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
 * @c job.Filter<VMAF>(log, "vmaf_4k_v0.6.1") or
 * @c job.Filter<VMAF>(log, "vmaf_4k_v0.6.1", 8).
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @class VMAF
	 * @brief Full-reference VMAF. Reference Analytics leaf.
	 *
	 * @par What an Analytics leaf is
	 * Analytics is not on the encode path. @ref Process sees
	 * only @ref Pipeline::Frame. Reference
	 * @ref Item::Producer is @ref Producer::Decoder.
	 * Distorted Producer is @ref Producer::Encoder or
	 * @ref Producer::Remuxer. There is no Packet and no
	 * @ref FFmpeg::Save.
	 *
	 * Implement @ref Setup, @ref Process, @ref Eof, @ref Clean,
	 * @ref Report, @ref Media.
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
	 * Process must not stall the encode tube: it clones the
	 * look, enqueues @c vmaf_read_pictures, and returns.
	 * Waiting for extractors is only in @ref Eof.
	 * @ref InputCeiling (512) sizes the analytics hopper, not
	 * the park. Parked AVFrames cover encoder delay (ref
	 * waiting for dist). libvmaf feature extractors scale
	 * with n_threads × resolution, not duration.
	 * Default n_threads is
	 * @c std::thread::hardware_concurrency() (all cores).
	 * 4K 10-bit (`vmaf_4k_v0.6.1`, subsample 1) at 32
	 * threads holds ~18.5 GiB for the extractors (massif
	 * plateau; peak ~20.5 GiB at init). RSS is the same
	 * order. A second video track opens another context of
	 * that size. Pass a smaller thread count in the
	 * constructor to cap it. Notice logs park depth every
	 * 64 scored pairs.
	 *
	 * A failed @c vmaf_read_pictures still belongs to us:
	 * Score unrefs both pictures.
	 *
	 * @par When to read @ref Report
	 * Muxer closed is not Eof on this node. Wait until the
	 * job is Done. A low score is not Fail. @ref Report::Failed
	 * only when a context or model could not be opened, or
	 * no pair was scored. One track keeps flat keys
	 * (`vmaf_mean`). Several tracks prefix with the origin
	 * index (`0.vmaf_mean`, `1.vmaf_mean`).
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::Analytics
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC VMAF: public Filter::Analytics {
		public:
			/**
			 * @brief VMAF with a built-in libvmaf model version.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param model libvmaf version key (`vmaf_4k_v0.6.1`).
			 * @param threads libvmaf worker count. Empty uses all
			 *        cores (`std::thread::hardware_concurrency()`).
			 *
			 * RAM follows threads × resolution, not duration. 4K
			 * 10-bit (`vmaf_4k_v0.6.1`, subsample 1) at 32 threads
			 * holds ~18.5 GiB for the job (peak ~20.5 GiB). A
			 * second video track opens another context of the
			 * same size.
			 */
			VMAF(std::shared_ptr<StormByte::Logger::Log> log, std::string model,
				std::optional<unsigned short> threads = {}) noexcept;

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
			 * @return Max items in the hopper. Never 0.
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
				std::size_t peakRef = 0;
				std::size_t peakDist = 0;
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
			 * On a failed read, the caller still owns both
			 * pictures and must unref them.
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
			 * @brief libvmaf worker count actually used (at least 1).
			 */
			unsigned Threads() const noexcept;

			/**
			 * @brief Notice park depth every @ref ParkLogEvery scored pairs.
			 */
			void LogPark(const Lane& lane, int track) noexcept;

			/**
			 * @brief Frees parked clones of @p lane.
			 */
			void DropParked(Lane& lane) noexcept;

			/**
			 * @brief Closes libvmaf and drops parked looks of every lane.
			 */
			void DropAll() noexcept;

			static constexpr std::size_t Ceiling = 512;	///< Analytics hopper
			static constexpr unsigned ParkLogEvery = 64;	///< Notice park depth
			std::string m_modelName;					///< libvmaf built-in version
			std::optional<unsigned short> m_threads;	///< Empty: all cores
			std::map<int, Lane> m_lanes;				///< One context per Frame::Track
	};
}
