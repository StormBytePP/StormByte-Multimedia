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

#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Demux
 * @brief Private demux backends behind the public Demux type.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Demux {
	/**
	 * @class Engine
	 * @brief Abstract demux backend. One instance per public Demux.
	 *
	 * The public Demux is the entry point. Open and Read live here.
	 * Details::Container implements this. Per-media decode is Decoder::Details.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Engine {
		public:
			/**
			 * @brief Destructor.
			 */
			virtual ~Engine() noexcept = default;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Engine(const Engine&) = delete;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Engine& operator=(const Engine&) = delete;

			/**
			 * @brief Whether the format context is open.
			 * @return true after a successful Open().
			 */
			virtual bool IsOpen() const noexcept = 0;

			/**
			 * @brief Opens the source File into this backend.
			 * @param owner Public demuxer (Fail).
			 * @param file Probed snapshot.
			 * @return false if owner.Fail() was called.
			 */
			virtual bool Open(class Demux& owner, const File& file) noexcept = 0;

			/**
			 * @brief Reads one compressed packet. TryAgain loops inside.
			 * @param owner Public demuxer (Fail, Eof).
			 * @param packet Replaced on success.
			 * @return true if @p packet was filled. false on EOF or Fail.
			 */
			virtual bool Read(class Demux& owner, Packet& packet) noexcept = 0;

			/**
			 * @brief Opened format context, if any. Used by demux >> decoder / copy.
			 * @return Context pointer, or nullptr.
			 */
			virtual void* Context() noexcept = 0;

		protected:
			/**
			 * @brief Default constructor.
			 */
			Engine() noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Engine to take.
			 */
			Engine(Engine&&) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Engine to take.
			 * @return *this.
			 */
			Engine& operator=(Engine&&) noexcept = default;
	};
}
