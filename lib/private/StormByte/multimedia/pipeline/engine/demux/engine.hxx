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
#include <StormByte/multimedia/pipeline/demuxer.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <memory>

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
	 * Errors go through @p owner.Fail. @ref Read returns an empty
	 * pointer at EoF.
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
			 * @brief Copy constructor.
			 * @param other Source engine.
			 */
			Engine(const Engine& other) = delete;

			/**
			 * @brief Copy assignment.
			 * @param other Source engine.
			 * @return *this.
			 */
			Engine& operator=(const Engine& other) = delete;

			/**
			 * @brief Whether the format context is open.
			 * @return true after a successful open.
			 */
			virtual bool IsOpen() const noexcept = 0;

			/**
			 * @brief Opens the source File into this backend.
			 * @param owner Public demuxer (Fail).
			 * @param file Probed snapshot.
			 * @return false if owner.Fail() was called.
			 */
			virtual bool Open(class StormByte::Multimedia::Pipeline::Demuxer& owner,
				const File& file) noexcept = 0;

			/**
			 * @brief Reads one compressed packet.
			 * @param owner Public demuxer (Fail, Eof).
			 * @return Packet with @ref Producer::Demux, or empty at EoF.
			 */
			virtual std::shared_ptr<StormByte::Multimedia::Pipeline::Packet> Read(
				class StormByte::Multimedia::Pipeline::Demuxer& owner) noexcept = 0;

			/**
			 * @brief Opened format context, if any.
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
			Engine(Engine&& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Engine to take.
			 * @return *this.
			 */
			Engine& operator=(Engine&& other) noexcept = default;
	};
}
