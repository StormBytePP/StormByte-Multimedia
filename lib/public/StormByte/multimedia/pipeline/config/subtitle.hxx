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

#include <StormByte/multimedia/codec.hxx>
#include <StormByte/multimedia/pipeline/config/base.hxx>
#include <StormByte/multimedia/visibility.h>

#include <map>
#include <string>
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
	class STORMBYTE_MULTIMEDIA_PUBLIC Subtitle: public Base {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Empty subtitle config (Remux until @ref Codec is set).
			 */
			constexpr Subtitle() noexcept
			: Base(StormByte::Multimedia::Type::Subtitle), m_codec(nullptr) {}

			/**
			 * @brief Copy constructor.
			 * @param other Source config.
			 */
			Subtitle(const Subtitle& other) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Config to take.
			 */
			Subtitle(Subtitle&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			virtual ~Subtitle() noexcept override = default;

			/**
			 * @brief Copy assignment.
			 * @param other Source config.
			 * @return *this.
			 */
			Subtitle& operator=(const Subtitle& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Config to take.
			 * @return *this.
			 */
			Subtitle& operator=(Subtitle&& other) noexcept = default;

			/**
			 * @}
			 */

			/**
			 * @brief Deep copy.
			 * @return Owning pointer to a new @ref Subtitle.
			 */
			inline PointerType Clone() const override {
				return MakePointer<Subtitle>(*this);
			}

			/**
			 * @brief Move into a new pointer.
			 * @return Owning pointer to the moved @ref Subtitle.
			 */
			inline PointerType Move() override {
				return MakePointer<Subtitle>(std::move(*this));
			}

			/**
			 * @name Encode
			 * @{
			 */

			/**
			 * @brief Destination codec.
			 * @return Registry codec, or `nullptr` if this track is Remux.
			 */
			inline const StormByte::Multimedia::Codec* Codec() const noexcept {
				return m_codec;
			}

			/**
			 * @brief Sets the destination codec (Encode).
			 * @param codec Registry codec. Must be subtitle at Plan validation.
			 */
			inline void Codec(const StormByte::Multimedia::Codec& codec) noexcept {
				m_codec = &codec;
			}

			/**
			 * @brief Vendor leftovers.
			 * @return Key/value map.
			 */
			inline const std::map<std::string, std::string>& FineTune() const noexcept {
				return m_fineTune;
			}

			/**
			 * @brief Replaces the vendor dict.
			 * @param options Key/value pairs.
			 */
			inline void FineTune(std::map<std::string, std::string> options) noexcept {
				m_fineTune = std::move(options);
			}

			/**
			 * @}
			 */

		private:
			const StormByte::Multimedia::Codec* m_codec;			///< Destination codec; nullptr = Remux
			std::map<std::string, std::string> m_fineTune;			///< Vendor leftovers
	};
}
