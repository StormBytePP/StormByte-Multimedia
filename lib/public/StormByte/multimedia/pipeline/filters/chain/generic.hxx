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

#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/visibility.h>

#include <concepts>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline::Filter::Chain
 * @brief Ordered lists of filter nodes owned by a pipeline.
 */
namespace StormByte::Multimedia::Pipeline::Filter::Chain {
	/**
	 * @class Generic
	 * @brief Storage, Add/Size and Fail state for one kind of
	 *        @ref StormByte::Multimedia::Pipeline::Filter::FFmpeg node.
	 *
	 * @tparam Node @ref Process, @ref Analytics or @ref Packet.
	 *
	 * Does not Push. @ref Fail returns false so a derived @c Push can
	 * @c return Fail(...). Not instantiated by user code.
	 */
	template<typename Node>
	requires std::derived_from<Node, StormByte::Multimedia::Pipeline::Filter::FFmpeg>
	class STORMBYTE_MULTIMEDIA_PUBLIC Generic {
		public:
			/**
			 * @brief Empty chain.
			 */
			Generic() noexcept = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Generic(const Generic&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Generic(Generic&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~Generic() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Generic& operator=(const Generic&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Generic& operator=(Generic&&) noexcept = default;

			/**
			 * @brief Appends a node. No-op if already @ref Failed or @p node is null.
			 * @param node Owned node.
			 */
			void Add(std::unique_ptr<Node> node) noexcept {
				if (m_failed || !node)
					return;
				m_nodes.push_back(std::move(node));
			}

			/**
			 * @brief Constructs and appends a node.
			 * @tparam FilterType Type derived from @p Node.
			 * @param args Constructor arguments.
			 * @return *this.
			 */
			template<typename FilterType, typename... Args>
			requires std::derived_from<FilterType, Node>
			Generic& Add(Args&&... args) noexcept {
				Add(std::make_unique<FilterType>(std::forward<Args>(args)...));
				return *this;
			}

			/**
			 * @brief Number of nodes.
			 * @return Count.
			 */
			std::size_t Size() const noexcept {
				return m_nodes.size();
			}

			/**
			 * @brief Whether @ref Fail was called.
			 * @return true after a hard error.
			 */
			bool Failed() const noexcept {
				return m_failed;
			}

			/**
			 * @brief Failure text.
			 * @return Message, or empty.
			 */
			const std::optional<std::string>& Error() const noexcept {
				return m_error;
			}

			/**
			 * @brief Marks a hard error.
			 * @param reason Message.
			 * @return false.
			 */
			bool Fail(std::string reason) noexcept {
				m_failed = true;
				m_error = std::move(reason);
				return false;
			}

		protected:
			std::vector<std::unique_ptr<Node>> m_nodes;	///< Nodes in declaration order

		private:
			bool m_failed = false;					///< Hard error
			std::optional<std::string> m_error;		///< Failure text
	};
}
