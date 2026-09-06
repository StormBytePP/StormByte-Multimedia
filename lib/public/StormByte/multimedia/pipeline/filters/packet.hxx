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

#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <optional>
#include <string>
#include <utility>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter
 * @brief User-supplied packet steps run by Demux.
 */
namespace StormByte::Multimedia::Pipeline::Filter {
	/**
	 * @class Packet
	 * @brief One packet-to-packet step. Inherit and implement Push.
	 *
	 * Return the same or a new Pipeline::Packet. Return empty optional
	 * to absorb it. On a hard error call Fail() and return empty.
	 * Push must not throw.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Packet {
		public:
			/**
			 * @brief Default constructor.
			 */
			Packet() noexcept = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Packet(const Packet&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Packet(Packet&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			virtual ~Packet() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Packet& operator=(const Packet&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Packet& operator=(Packet&&) noexcept = default;

			/**
			 * @brief Runs this step on @p packet.
			 * @param packet Incoming access unit (moved in).
			 * @return Outgoing access unit, or empty if absorbed or failed.
			 */
			virtual std::optional<Pipeline::Packet> Push(Pipeline::Packet&& packet) noexcept = 0;

			/**
			 * @brief Whether Push set a hard error.
			 * @return true after Fail().
			 */
			bool Failed() const noexcept {
				return m_failed;
			}

			/**
			 * @brief Failure text, if Failed().
			 * @return Message, or empty.
			 */
			const std::optional<std::string>& Error() const noexcept {
				return m_error;
			}

		protected:
			/**
			 * @brief Marks a hard error. Further Push calls should no-op.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept {
				m_failed = true;
				m_error = std::move(reason);
			}

		private:
			bool m_failed = false;				///< Hard error
			std::optional<std::string> m_error;		///< Failure text
	};
}
