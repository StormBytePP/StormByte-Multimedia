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

#include <StormByte/buffer/consumer.hxx>
#include <StormByte/multimedia/visibility.h>

#include <filesystem>
#include <utility>
#include <variant>

/**
 * @namespace StormByte::Multimedia
 * @brief Public multimedia types: codecs, containers, streams and files.
 */
namespace StormByte::Multimedia {
	/**
	 * @class Origin
	 * @brief Input source for File and Demux: filesystem path or Consumer.
	 *
	 * Private type. Not installed. Not thread-safe beyond Consumer's own rules.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Origin {
		public:
			/**
			 * @brief Path origin.
			 * @param path Media file.
			 */
			explicit Origin(std::filesystem::path path) noexcept
			: m_held(std::move(path)) {}

			/**
			 * @brief Consumer origin.
			 * @param consumer Shared ring handle (copied).
			 */
			explicit Origin(StormByte::Buffer::Consumer consumer) noexcept
			: m_held(std::move(consumer)) {}

			/**
			 * @brief Copy constructor (deleted).
			 */
			Origin(const Origin&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Origin(Origin&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Origin() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Origin& operator=(const Origin&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Origin& operator=(Origin&&) noexcept = default;

			/**
			 * @brief Filesystem path if this origin is a file.
			 * @return Path, or nullptr.
			 */
			const std::filesystem::path* Path() const noexcept {
				return std::get_if<std::filesystem::path>(&m_held);
			}

			/**
			 * @brief Consumer if this origin is a buffer.
			 * @return Consumer, or nullptr.
			 */
			StormByte::Buffer::Consumer* Consumer() noexcept {
				return std::get_if<StormByte::Buffer::Consumer>(&m_held);
			}

			/**
			 * @brief Consumer if this origin is a buffer.
			 * @return Consumer, or nullptr.
			 */
			const StormByte::Buffer::Consumer* Consumer() const noexcept {
				return std::get_if<StormByte::Buffer::Consumer>(&m_held);
			}

			/**
			 * @brief Applies @p fn to the held path or Consumer.
			 * @tparam Fn Visitor.
			 * @param fn Visitor.
			 * @return Visitor result.
			 */
			template<typename Fn>
			decltype(auto) Visit(Fn&& fn) const {
				return std::visit(std::forward<Fn>(fn), m_held);
			}

			/**
			 * @brief Applies @p fn to the held path or Consumer.
			 * @tparam Fn Visitor.
			 * @param fn Visitor.
			 * @return Visitor result.
			 */
			template<typename Fn>
			decltype(auto) Visit(Fn&& fn) {
				return std::visit(std::forward<Fn>(fn), m_held);
			}

		private:
			std::variant<std::filesystem::path, StormByte::Buffer::Consumer> m_held;	///< Path or Consumer
	};
}
