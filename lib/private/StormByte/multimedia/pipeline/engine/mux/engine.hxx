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
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/mux.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <filesystem>

/**
 * @namespace StormByte::Multimedia::Pipeline::Engine::Mux
 * @brief Private mux backends behind the public Mux type.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline::Engine::Mux {
	/**
	 * @class Engine
	 * @brief Abstract mux backend. One instance per public Mux.
	 *
	 * The public Mux is the entry point. Path, tracks, packets, flush and
	 * attachments live in Details::Container / Details::Attachment.
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
			 * @brief Whether a destination file is bound.
			 * @return true after mux >> path.
			 */
			virtual bool IsOpen() const noexcept = 0;

			/**
			 * @brief Whether avformat_write_header has run.
			 * @return true after a successful header.
			 */
			virtual bool HeaderWritten() const noexcept = 0;

			/**
			 * @brief Binds the destination path and allocates the format context.
			 * @param owner Public muxer (Fail, Destination).
			 * @param path Output file.
			 * @return false if owner.Fail() was called.
			 */
			virtual bool BindPath(class Mux& owner, const std::filesystem::path& path) noexcept = 0;

			/**
			 * @brief Reserves an encode track at Encoder::Index().
			 * @param owner Public muxer.
			 * @param encoder Live encoder (must outlive the header).
			 * @return false if owner.Fail() was called.
			 */
			virtual bool ReserveEncoder(class Mux& owner, class Encoder& encoder) noexcept = 0;

			/**
			 * @brief Reserves a copy track at Copy::Index().
			 * @param owner Public muxer.
			 * @param copy Bound copy handle.
			 * @return false if owner.Fail() was called.
			 */
			virtual bool ReserveCopy(class Mux& owner, const class Copy& copy) noexcept = 0;

			/**
			 * @brief Snapshots File attachments for header time.
			 * @param owner Public muxer.
			 * @param file Source file.
			 * @return false if owner.Fail() was called.
			 */
			virtual bool BindAttachments(class Mux& owner, const File& file) noexcept = 0;

			/**
			 * @brief Queues or writes one packet. Writes the header when ready.
			 * @param owner Public muxer.
			 * @param packet Encoded or copied packet.
			 * @return false if owner.Fail() was called.
			 */
			virtual bool Push(class Mux& owner, Packet& packet) noexcept = 0;

			/**
			 * @brief Flushes reserved encoders, leftover packets and the trailer.
			 * @param owner Public muxer.
			 */
			virtual void Flush(class Mux& owner) noexcept = 0;

			/**
			 * @brief Writes trailer if needed and frees AVIO + context.
			 */
			virtual void Close() noexcept = 0;

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
