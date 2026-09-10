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

#include <cstdint>
#include <map>
#include <optional>
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
	class STORMBYTE_MULTIMEDIA_PUBLIC Video: public Base {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Empty video config (Remux until @ref Codec is set).
			 */
			constexpr Video() noexcept
			: Base(StormByte::Multimedia::Type::Video), m_codec(nullptr) {}

			/**
			 * @brief Copy constructor.
			 * @param other Source config.
			 */
			Video(const Video& other) noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Config to take.
			 */
			Video(Video&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			virtual ~Video() noexcept override = default;

			/**
			 * @brief Copy assignment.
			 * @param other Source config.
			 * @return *this.
			 */
			Video& operator=(const Video& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Config to take.
			 * @return *this.
			 */
			Video& operator=(Video&& other) noexcept = default;

			/**
			 * @}
			 */

			/**
			 * @brief Deep copy.
			 * @return Owning pointer to a new @ref Video.
			 */
			inline PointerType Clone() const override {
				return MakePointer<Video>(*this);
			}

			/**
			 * @brief Move into a new pointer.
			 * @return Owning pointer to the moved @ref Video.
			 */
			inline PointerType Move() override {
				return MakePointer<Video>(std::move(*this));
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
			 * @param codec Registry codec. Must be video at Plan validation.
			 */
			inline void Codec(const StormByte::Multimedia::Codec& codec) noexcept {
				m_codec = &codec;
			}

			/**
			 * @brief CRF/CQ. Incompatible with BitRate.
			 * @return Value, or empty.
			 */
			inline const std::optional<int>& CRF() const noexcept {
				return m_crf;
			}

			/**
			 * @brief Sets CRF/CQ and clears BitRate.
			 * @param value Quality value.
			 */
			void CRF(int value) noexcept;

			/**
			 * @brief Target bitrate.
			 * @return Bits per second, or empty.
			 */
			inline const std::optional<std::int64_t>& BitRate() const noexcept {
				return m_bitRate;
			}

			/**
			 * @brief Sets target bitrate and clears CRF.
			 * @param bits_per_second Bits per second.
			 */
			void BitRate(std::int64_t bits_per_second) noexcept;

			/**
			 * @brief VBV ceiling.
			 * @return Bits per second, or empty.
			 */
			inline const std::optional<std::int64_t>& MaxBitRate() const noexcept {
				return m_maxBitRate;
			}

			/**
			 * @brief Sets VBV ceiling. `bufsize` is derived by the encoder.
			 * @param bits_per_second Max bitrate.
			 */
			inline void MaxBitRate(std::int64_t bits_per_second) noexcept {
				m_maxBitRate = bits_per_second;
			}

			/**
			 * @brief Encoder preset.
			 * @return Name, or empty.
			 */
			inline const std::optional<std::string>& Preset() const noexcept {
				return m_preset;
			}

			/**
			 * @brief Sets the preset. Empty clears it.
			 * @param name Preset name (`medium`, `p4`, …).
			 */
			void Preset(std::string name) noexcept;

			/**
			 * @brief Content tune.
			 * @return Name, or empty.
			 */
			inline const std::optional<std::string>& Tune() const noexcept {
				return m_tune;
			}

			/**
			 * @brief Sets the tune. Empty clears it.
			 * @param name Tune name (`animation`, `film`, …).
			 */
			void Tune(std::string name) noexcept;

			/**
			 * @brief Vendor leftovers. Not CRF / preset / tune / bufsize.
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
			std::optional<int> m_crf;								///< CRF/CQ
			std::optional<std::int64_t> m_bitRate;					///< Target bitrate
			std::optional<std::int64_t> m_maxBitRate;				///< VBV ceiling
			std::optional<std::string> m_preset;					///< Preset
			std::optional<std::string> m_tune;						///< Tune
			std::map<std::string, std::string> m_fineTune;			///< Vendor leftovers
	};
}
