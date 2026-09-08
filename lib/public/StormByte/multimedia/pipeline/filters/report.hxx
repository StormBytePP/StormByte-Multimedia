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

#include <map>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter
 * @brief Frame and packet steps attached to a job or a raw pipeline.
 */
namespace StormByte::Multimedia::Pipeline::Filter {
	/**
	 * @class Report
	 * @brief Optional measurement produced by a filter.
	 *
	 * Status is measurement-only. Quality thresholds belong in
	 * @c Transcode::OnReport and @c Transcode::ExtraData, not here.
	 *
	 * @see StormByte::Multimedia::Pipeline::Filter::FFmpeg
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Report {
		public:
			/**
			 * @enum Status
			 * @brief Whether this node produced usable data.
			 */
			enum class Status {
				None,		///< Filter does not report
				Ok,		///< Data() is usable
				Failed		///< Measurement could not be taken
			};

			/**
			 * @brief Empty report (`None`).
			 */
			Report() noexcept;

			/**
			 * @brief Report with a status and a dictionary.
			 * @param status Measurement result.
			 * @param data Key/value payload (owned).
			 */
			Report(Status status, std::map<std::string, std::string> data) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source report.
			 */
			Report(const Report& other) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Source report.
			 */
			Report(Report&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Report() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @param other Source report.
			 * @return *this.
			 */
			Report& operator=(const Report& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Source report.
			 * @return *this.
			 */
			Report& operator=(Report&& other) noexcept = default;

			/**
			 * @brief Measurement status.
			 * @return Status value.
			 */
			Status Kind() const noexcept;

			/**
			 * @brief Dictionary payload.
			 * @return Owned key/value map (may be empty).
			 */
			const std::map<std::string, std::string>& Data() const noexcept;

			/**
			 * @brief Single callable dump of status plus data.
			 * @return Human-readable snapshot.
			 */
			std::string operator()() const noexcept;

		private:
			Status m_status;					///< Measurement status
			std::map<std::string, std::string> m_data;	///< Payload
	};
}
