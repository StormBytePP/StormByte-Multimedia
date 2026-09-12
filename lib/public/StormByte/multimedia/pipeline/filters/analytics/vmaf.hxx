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
 * @c job.Video(in, out).Filter<VMAF>(log, "vmaf_4k_v0.6.1").
 */
namespace StormByte::Multimedia::Pipeline::Filter::Video {
	/**
	 * @class VMAF
	 * @brief Full-reference VMAF. Reference Analytics leaf.
	 *
	 * @par What an Analytics leaf is
	 * Analytics is not on the encode path. @ref Route::Close Tees
	 * the Decoder (reference look) and feeds a Route-owned encode
	 * look (distorted look) onto this node. @ref Process sees only
	 * @ref Pipeline::Frame. @ref Item::Producer is
	 * @ref Producer::Decoder or @ref Producer::Encoder. There is
	 * no Packet and no @ref FFmpeg::Save — Analytics does not
	 * rewrite the backend.
	 *
	 * Implement @ref Setup, @ref Process, @ref Eof, @ref Clean,
	 * @ref Report, @ref Media. Route launches the worker.
	 *
	 * @par Pairing
	 * Presentation FIFOs, not @ref Item::Serial. Serial is tube
	 * lineage: Encoder stamps the last ingested frame on each
	 * packet, which is not the reconstructed picture when there
	 * are B-frames. Decode PTS and encode PTS are different
	 * clocks. Each look leaves avcodec in presentation order,
	 * so the nth Decoder frame is the same picture as the nth
	 * Encoder look frame.
	 *
	 * @par Geometry
	 * Latched on the first valid reference. Distorted looks of
	 * another size are scaled to that latch (Scale in the tube
	 * is the usual case). A later reference that changes
	 * width/height is skipped with a Warning — feeding libvmaf
	 * a second size aborts with
	 * `corrupted size vs. prev_size while consolidating`.
	 * Scale is not @ref Report::Failed.
	 *
	 * @par Memory
	 * @ref InputCeiling is the Tee hopper, not the park. Work
	 * pops the hopper and this leaf clones into @c m_ref /
	 * @c m_dist. A ceiling of 8 on the hopper does nothing if
	 * Process drains it into an unbounded deque. Parked decode
	 * looks must cover encoder delay (filters + libx265
	 * lookahead). Too small and the first distorted frame
	 * never arrives. After the look starts, @ref Drain keeps
	 * both queues near that delay, not the whole feature.
	 *
	 * @par When to read @ref Report
	 * Muxer closed is not Eof on this node. The encode look
	 * lags. @ref Report before @ref Eof has no pooled mean
	 * even if hundreds of pairs already went to libvmaf.
	 * Halt the Route (filters + look Decoders) first. Same
	 * rule for a hand-built tube.
	 *
	 * libvmaf built-in model via @c vmaf_model_load (`version=`
	 * in the ffmpeg filter). Example: `vmaf_4k_v0.6.1`.
	 *
	 * A low score is not Fail. @ref Report::Failed only when
	 * the context or model could not be opened, or no pair
	 * was scored.
	 *
	 * The first constructor argument is the shared logger of
	 * the tube. The leaf may call protected @ref FFmpeg::Log.
	 * See [StormByte Logger](https://dev.stormbyte.org/StormByte-Logger/).
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::Analytics
	 * @see StormByte::Multimedia::Pipeline::Route
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC VMAF: public Filter::Analytics {
		public:
			/**
			 * @brief VMAF with a built-in libvmaf model version.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param model libvmaf version key (`vmaf_4k_v0.6.1`,
			 *        `vmaf_v0.6.1`). Must be a built-in model.
			 */
			VMAF(std::shared_ptr<StormByte::Logger::Log> log, std::string model) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source filter.
			 */
			VMAF(const VMAF& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Filter to take.
			 */
			VMAF(VMAF&& other) noexcept = delete;

			/**
			 * @brief Destructor. Closes libvmaf and drops parked looks.
			 */
			~VMAF() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source filter.
			 * @return *this.
			 */
			VMAF& operator=(const VMAF& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Filter to take.
			 * @return *this.
			 */
			VMAF& operator=(VMAF&& other) noexcept = delete;

			/**
			 * @brief Media this filter handles.
			 * @return Video. Other kinds are ignored in Process.
			 */
			enum StormByte::Multimedia::Type Media() const noexcept override;

			/**
			 * @brief Ceiling of the analytics input hopper.
			 * @return Max items in the Tee / look hoppers. Never 0.
			 *
			 * Must be larger than encoder delay or the first
			 * distorted look cannot arrive. Not the 3-hour bound:
			 * that is @ref Drain after the look starts.
			 */
			std::size_t InputCeiling() const noexcept override {
				return Ceiling;
			}

			/**
			 * @brief Pooled VMAF after EoF.
			 * @return Ok with mean/min/frames/model/size, or Failed.
			 */
			class Filter::Report Report() const noexcept override;

		protected:
			/**
			 * @brief Drops libvmaf state and parked looks.
			 */
			void Clean() noexcept override;

			/**
			 * @brief Opens libvmaf and loads the model version.
			 */
			void Setup() noexcept override;

			/**
			 * @brief Parks a look and scores when both FIFOs have a head.
			 * @param frame Current look. Never null.
			 *
			 * Decoder → @c m_ref. Encoder → @c m_dist. Then @ref Drain.
			 */
			void Process(const Pipeline::Frame& frame) noexcept override;

			/**
			 * @brief Flushes libvmaf and pools the score.
			 *
			 * Callers that read @ref Report before this runs see Failed
			 * with an empty mean.
			 */
			void Eof() noexcept override;

		private:
			/**
			 * @brief Copies @p raw into a VmafPicture, scaling to @p tw x @p th when needed.
			 * @param raw Source libav frame.
			 * @param tw Target width (the latch).
			 * @param th Target height (the latch).
			 * @param out Allocated picture. Caller unrefs after vmaf_read_pictures.
			 * @return false on unknown pixfmt, alloc or scale failure.
			 */
			bool Fill(const ::AVFrame* raw, int tw, int th, void* out) noexcept;

			/**
			 * @brief Scores one paired look.
			 * @param ref Decoder frame.
			 * @param dist Encoder frame.
			 * @param index libvmaf picture index. Contiguous successes only.
			 */
			void Score(const ::AVFrame* ref, const ::AVFrame* dist, unsigned index) noexcept;

			/**
			 * @brief Scores while both presentation FIFOs have a frame.
			 */
			void Drain() noexcept;

			/**
			 * @brief Releases every parked AVFrame.
			 */
			void DropParked() noexcept;

			static constexpr std::size_t Ceiling = 512;	///< Hopper + parked looks; must exceed encode delay
			std::string m_modelName;					///< libvmaf built-in version
			VmafContext* m_vmaf;						///< libvmaf context
			VmafModel* m_model;							///< Loaded model
			std::deque<::AVFrame*> m_ref;				///< Decoder looks, presentation order
			std::deque<::AVFrame*> m_dist;				///< Encoder looks, presentation order
			int m_width;								///< Latched picture width
			int m_height;								///< Latched picture height
			unsigned m_index;							///< Next contiguous libvmaf index
			unsigned m_scored;							///< Pairs accepted by libvmaf
			std::optional<double> m_mean;				///< Pooled mean after Eof
			std::optional<double> m_min;				///< Pooled min after Eof
			bool m_failed;								///< Context or model failed to open
	};
}
