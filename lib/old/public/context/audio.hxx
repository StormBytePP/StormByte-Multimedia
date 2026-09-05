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

#include <StormByte/multimedia/context/generic.hxx>

#include <string>
#include <optional>

/**
 * @namespace Context
 * @brief Media stream context types (audio, video, …).
 */
namespace StormByte::Multimedia::Context {
	/**
	 * @class Audio
	 * @brief Audio stream context (sample rate, channels, bitrate, profile).
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Audio final: public Generic {
		public:
			/**
			 * @param sample_rate Sample rate in Hz.
			 * @param channels Channel count.
			 * @param bitrate Bitrate in bits/s (0 if unknown).
			 * @param profile Optional codec profile name.
			 */
			Audio(unsigned int sample_rate, unsigned short channels, unsigned int bitrate,
				const std::optional<std::string>& profile = std::nullopt) noexcept;

			/**
			 * Copy constructor.
			 */
			Audio(const Audio& other) = default;

			/**
			 * Move constructor.
			 */
			Audio(Audio&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~Audio() noexcept = default;

			/**
			 * Copy assignment.
			 */
			Audio& operator=(const Audio& other) = default;

			/**
			 * Move assignment.
			 */
			Audio& operator=(Audio&& other) = default;

			/**
			 * @return Sample rate in Hz.
			 */
			unsigned int SampleRate() const noexcept;

			/**
			 * @return Number of channels.
			 */
			unsigned short Channels() const noexcept;

			/**
			 * @return Bitrate in bits/s.
			 */
			unsigned int Bitrate() const noexcept;

			/**
			 * @return Optional profile name.
			 */
			const std::optional<std::string>& Profile() const noexcept;

			/**
			 * @return Cloned context.
			 */
			PointerType Clone() const override;

			/**
			 * @return Moved context as new pointer.
			 */
			PointerType Move() override;

		private:
			unsigned int m_sample_rate;					///< Sample rate (Hz)
			unsigned short m_channels;					///< Channel count
			unsigned int m_bitrate;						///< Bitrate (bits/s)
			std::optional<std::string> m_profile;		///< Optional profile
	};
}
