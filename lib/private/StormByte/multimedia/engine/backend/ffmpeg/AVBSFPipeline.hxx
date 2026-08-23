/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte-Multimedia.
 *
 * StormByte-Multimedia is dual-licensed under the following terms:
 *
 * 1. GNU Lesser General Public License v3.0 (or later)
 *    You can redistribute it and/or modify it under the terms of the
 *    GNU Lesser General Public License as published by the Free Software
 *    Foundation, either version 3 of the License, or (at your option)
 *    any later version.
 *
 * 2. Commercial license
 *    Alternatively, this software may be used under the terms of a
 *    commercial license agreement with the sole copyright holder
 *    (David C. Manuelda <StormByte@gmail.com>).
 *    Contact the copyright holder for more information.
 *
 * StormByte-Multimedia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this StormByte-Multimedia. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/multimedia/engine/backend/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/typedefs.hxx>

#include <deque>

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVPacket;

	/**
	 * @class AVBSFPipeline
	 * @brief Ordered chain of bitstream filters applied to packets.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVBSFPipeline {
		public:
			/**
			 * Default constructor.
			 */
			AVBSFPipeline() noexcept = default;

			/**
			 * Copy constructor (deleted).
			 */
			AVBSFPipeline(const AVBSFPipeline&) = delete;

			/**
			 * Move constructor.
			 */
			AVBSFPipeline(AVBSFPipeline&& other) noexcept;

			/**
			 * Destructor.
			 */
			~AVBSFPipeline() noexcept = default;

			/**
			 * Copy assignment (deleted).
			 */
			AVBSFPipeline& operator=(const AVBSFPipeline&) = delete;

			/**
			 * Move assignment.
			 */
			AVBSFPipeline& operator=(AVBSFPipeline&& other) noexcept;

			/**
			 * Appends a filter to the chain.
			 * @param bsf Filter to take ownership of.
			 */
			void Add(AVBSF&& bsf) noexcept;

			/**
			 * Runs @p pkt through all filters in order.
			 * @param pkt Packet in/out.
			 * @return Operation result.
			 */
			OperationResult Process(AVPacket& pkt) noexcept;

			/**
			 * Flushes every filter.
			 */
			void Flush() noexcept;

			/**
			 * Signals EOF on every filter.
			 */
			void SetEof() noexcept;

			/**
			 * Removes all filters.
			 */
			void Clear() noexcept;

			/**
			 * @return true if no filters are registered.
			 */
			bool Empty() const noexcept;

		private:
			std::deque<AVBSF> m_filters;	///< Filter chain
	};
}
