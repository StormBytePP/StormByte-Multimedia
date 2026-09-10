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

#include <StormByte/multimedia/pipeline/config/base.hxx>
#include <StormByte/multimedia/visibility.h>

#include <string>
#include <string_view>
#include <utility>

/**
 * @namespace StormByte::Multimedia::Pipeline::Config
 * @brief Per-track intention stored by Plan.
 *
 * Not wiring (`operator>>`) and not runtime settled state.
 * An engaged `std::optional` is an explicit override. Calling a
 * setter with an empty value is also an explicit override.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Config {
	/**
	 * @class Attachment
	 * @brief Attachment-track intention.
	 *
	 * One leaf per attached stream. There is no “all attachments”
	 * config; omit the track to drop, @ref Track with this leaf to
	 * keep it. Tags live on @ref Base.
	 *
	 * @ref MimeType is required. Wildcards (`image/*`) are not valid.
	 *
	 * @todo Replace @ref MimeType with an enum.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Attachment: public Base {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Builds from a mime view (copied).
			 * @param mime_type Concrete MIME type (`image/jpeg`, `font/ttf`, …).
			 */
			explicit Attachment(std::string_view mime_type) noexcept
			: Base(StormByte::Multimedia::Type::Attachment), m_mimeType(mime_type) {}

			/**
			 * @brief Builds from a moved mime string.
			 * @param mime_type Concrete MIME type.
			 */
			explicit Attachment(std::string&& mime_type) noexcept
			: Base(StormByte::Multimedia::Type::Attachment), m_mimeType(std::move(mime_type)) {}

			/**
			 * @brief Copy constructor.
			 * @param other Source config.
			 */
			Attachment(const Attachment& other) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Config to take.
			 */
			Attachment(Attachment&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			virtual ~Attachment() noexcept override = default;

			/**
			 * @brief Copy assignment.
			 * @param other Source config.
			 * @return *this.
			 */
			Attachment& operator=(const Attachment& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Config to take.
			 * @return *this.
			 */
			Attachment& operator=(Attachment&& other) noexcept = default;

			/**
			 * @}
			 */

			/**
			 * @brief Deep copy.
			 * @return Owning pointer to a new @ref Attachment.
			 */
			inline PointerType Clone() const override {
				return MakePointer<Attachment>(*this);
			}

			/**
			 * @brief Move into a new pointer.
			 * @return Owning pointer to the moved @ref Attachment.
			 */
			inline PointerType Move() override {
				return MakePointer<Attachment>(std::move(*this));
			}

			/**
			 * @brief Concrete MIME type. Not a wildcard.
			 * @return MIME string.
			 */
			inline const std::string& MimeType() const noexcept {
				return m_mimeType;
			}

			/**
			 * @brief Replaces the MIME type (moved).
			 * @param mime_type Concrete MIME type. Not a wildcard.
			 */
			inline void MimeType(std::string&& mime_type) noexcept {
				m_mimeType = std::move(mime_type);
			}

			/**
			 * @brief Replaces the MIME type (copied from a view).
			 * @param mime_type Concrete MIME type. Not a wildcard.
			 */
			inline void MimeType(std::string_view mime_type) noexcept {
				m_mimeType = mime_type;
			}

		private:
			std::string m_mimeType;	///< Concrete MIME type; no wildcards
	};
}
