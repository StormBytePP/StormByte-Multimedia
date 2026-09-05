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

#include <StormByte/multimedia/property/channel_layout.hxx>
#include <StormByte/multimedia/visibility.h>

#include <cstdint>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Property
 * @brief Media property value types.
 */
namespace StormByte::Multimedia::Property {
	/**
	 * @class Audio
	 * @brief Per-stream audio properties.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Audio final {
		public:
			/**
			 * @brief Constructs audio properties.
			 * @param layout Speaker layout.
			 * @param sample_rate Sample rate in Hz.
			 * @param channels Channel count.
			 * @param bitrate Bitrate in bits per second (0 if unknown).
			 * @param profile Optional codec profile name.
			 */
			Audio(ChannelLayout layout, std::uint32_t sample_rate, std::uint8_t channels,
				std::uint64_t bitrate = 0, std::optional<std::string> profile = std::nullopt) noexcept;

			/**
			 * @brief Copy constructor.
			 */
			Audio(const Audio&) = default;

			/**
			 * @brief Move constructor.
			 */
			Audio(Audio&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Audio() noexcept = default;

			/**
			 * @brief Copy assignment.
			 * @return *this.
			 */
			Audio& operator=(const Audio&) = default;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Audio& operator=(Audio&&) noexcept = default;

			/**
			 * @brief Speaker layout.
			 * @return Layout.
			 */
			ChannelLayout Layout() const noexcept;

			/**
			 * @brief Sample rate in Hz.
			 * @return Sample rate.
			 */
			std::uint32_t SampleRate() const noexcept;

			/**
			 * @brief Channel count.
			 * @return Channel count.
			 */
			std::uint8_t Channels() const noexcept;

			/**
			 * @brief Bitrate in bits per second.
			 * @return Bitrate, or 0 if unknown.
			 */
			std::uint64_t BitRate() const noexcept;

			/**
			 * @brief Codec profile name, if present.
			 * @return Profile, or empty.
			 */
			const std::optional<std::string>& Profile() const noexcept;

		private:
			ChannelLayout m_layout;					///< Speaker layout
			std::uint32_t m_sample_rate;			///< Sample rate (Hz)
			std::uint8_t m_channels;				///< Channel count
			std::uint64_t m_bitrate;				///< Bitrate (bits/s)
			std::optional<std::string> m_profile;	///< Optional profile
	};
}
