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

#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Demuxer;
	class Filters;
	class Muxer;

	/**
	 * @class Progress
	 * @brief Job clock for one tube.
	 *
	 * The Demuxer creates one instance and shares it. Filters write
	 * analytics, the Muxer writes that the container finished.
	 * @ref Transcoder::Progress and @ref Demuxer::Progress return
	 * @ref Pointer (shared, const). The user may keep that pointer
	 * after the tube dies. There are no public setters.
	 *
	 * Public axes are measure (optional 2-pass) and analytics
	 * (optional taps). Ordinary Process has no public name; its
	 * score only enters @ref All.
	 *
	 * While measure is live, @ref All tracks measure. After
	 * MeasureDone, All does not drop; mux and analytics add on
	 * top. 100.00 only when the Muxer finished and every mounted
	 * phase is closed. Measure pts come from frames that already
	 * ran Measure, not from demux emission.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Progress {
		friend class Demuxer;
		friend class Filters;
		friend class Muxer;

		public:
			/**
			 * @brief User-facing handle. Const, shared. Not a raw pointer.
			 */
			using Pointer = std::shared_ptr<const Progress>;

			/**
			 * @brief Empty clock. No measure, no analytics.
			 */
			Progress() noexcept = default;

			/**
			 * @brief Copy constructor.
			 * @param other Source clock.
			 */
			Progress(const Progress& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Clock to take.
			 */
			Progress(Progress&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Progress() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @param other Source clock.
			 * @return *this.
			 */
			Progress& operator=(const Progress& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Clock to take.
			 * @return *this.
			 */
			Progress& operator=(Progress&& other) noexcept = delete;

			/**
			 * @brief First-pass percent, if the tube mounted 2-pass.
			 * @return 0..100, or empty if there is no measure pass.
			 */
			std::optional<double> Measure() const noexcept;

			/**
			 * @brief Dest-look percent, if analytics taps exist.
			 * @return 0..100, or empty if there are no taps.
			 */
			std::optional<double> Analytics() const noexcept;

			/**
			 * @brief Whether this tube mounted a measure pass.
			 * @return true after Demuxer::Measure with a non-empty track list.
			 */
			bool HasMeasure() const noexcept;

			/**
			 * @brief Whether this tube mounted analytics taps.
			 * @return true after Filters wired a dest look.
			 */
			bool HasAnalytics() const noexcept;

			/**
			 * @brief Whether the measure pass is finished.
			 * @return true if there is no measure pass, or it has closed.
			 */
			bool MeasureComplete() const noexcept;

			/**
			 * @brief Whether analytics is finished.
			 * @return true if there are no taps, or they are idle.
			 */
			bool AnalyticsComplete() const noexcept;

			/**
			 * @brief Combined job percent.
			 *
			 * While measure is live this equals @ref Measure.
			 * After that, mux and analytics continue from that
			 * floor. 100 only when the Muxer finished and every
			 * mounted phase is closed. Monotone.
			 *
			 * @return 0..100.
			 */
			double All() const noexcept;

			/**
			 * @brief One CR-safe line: live axes plus @ref All.
			 *
			 * An axis is omitted when it was not mounted or already
			 * finished. No newline.
			 *
			 * @return Single line, no trailing newline.
			 */
			operator std::string() const noexcept;

		private:
			/**
			 * @brief Records that this tube has a measure pass.
			 * @param on true when Demuxer::Measure was called.
			 */
			void HasMeasure(bool on) noexcept;

			/**
			 * @brief Records that this tube has analytics taps.
			 * @param on true when Filters wired a dest look.
			 */
			void HasAnalytics(bool on) noexcept;

			/**
			 * @brief Origin duration used as ceiling for every axis.
			 * @param ns Duration in nanoseconds. Ignored if not > 0.
			 */
			void SetDurationNs(std::int64_t ns) noexcept;

			/**
			 * @brief Measure-pass position. Monotone, clamped to duration.
			 * @param ns Last measured frame Pts.
			 */
			void SetMeasureNs(std::int64_t ns) noexcept;

			/**
			 * @brief Ordinary Process-pass position. Monotone, clamped to duration.
			 * @param ns Last packet Pts while not measuring.
			 */
			void SetPassNs(std::int64_t ns) noexcept;

			/**
			 * @brief Analytics position. Monotone, clamped to duration.
			 * @param ns Dest-look Pts.
			 */
			void SetAnalyticsNs(std::int64_t ns) noexcept;

			/**
			 * @brief Measure pass closed. Freezes measure at duration.
			 */
			void MeasureDone() noexcept;

			/**
			 * @brief Ordinary Process pass hit source EoF. Freezes that Pts at duration.
			 */
			void PassDone() noexcept;

			/**
			 * @brief Muxer finished writing the destination.
			 */
			void MuxDone() noexcept;

			/**
			 * @brief Analytics taps idle. Freezes analytics at duration.
			 */
			void AnalyticsDone() noexcept;

			/**
			 * @brief Maps a position to 0..100 against @p dur.
			 * @param pos Position in nanoseconds.
			 * @param dur Duration in nanoseconds.
			 * @return 0 if unusable; 100 if @p pos >= @p dur.
			 */
			static double Axis(std::int64_t pos, std::int64_t dur) noexcept;

			bool m_hasMeasure = false;			///< Tube mounted a 2-pass leaf
			bool m_hasAnalytics = false;		///< Tube mounted dest-look taps
			bool m_measureDone = false;			///< Measure pass closed
			bool m_passDone = false;			///< Ordinary Process pass at source EoF
			bool m_muxDone = false;				///< Muxer wrote trailer
			bool m_analyticsDone = false;		///< Analytics taps idle
			std::int64_t m_durationNs = 0;		///< Origin duration (ns)
			std::int64_t m_measureNs = 0;		///< Measure position (ns)
			std::int64_t m_passNs = 0;			///< Ordinary Process position (ns)
			std::int64_t m_analyticsNs = 0;		///< Analytics position (ns)
			std::int64_t m_analyticsAtMux = 0;	///< Analytics ns when MuxDone ran
			mutable double m_all = 0.0;			///< Last published All (monotone)
	};
}
