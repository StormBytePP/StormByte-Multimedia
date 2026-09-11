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
#include <StormByte/multimedia/pipeline/muxer.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <filesystem>
#include <memory>

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	/**
	 * @class Muxer
	 * @brief Mux backend behind one public Muxer.
	 *
	 * One leaf per destination container family. BindPath, ReserveEncoder,
	 * ReserveRemux and BindAttachments stay bool+Fail. Push uses shared_ptr.
	 * Errors are owner.Fail(). Remux reservation is an output index plus
	 * the origin stream index; the leaf fills codecpar from the bound File.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Muxer {
		public:
			/**
			 * @brief Destructor.
			 */
			virtual ~Muxer() noexcept = default;

			/**
			 * @brief Copy constructor.
			 * @param other Source backend.
			 */
			Muxer(const Muxer& other) = delete;

			/**
			 * @brief Copy assignment.
			 * @param other Source backend.
			 * @return *this.
			 */
			Muxer& operator=(const Muxer& other) = delete;

			/**
			 * @brief Whether a destination file is bound.
			 * @return true after muxer >> path.
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
			virtual bool BindPath(StormByte::Multimedia::Pipeline::Muxer& owner,
				const std::filesystem::path& path) noexcept = 0;

			/**
			 * @brief Reserves an encode track at Encoder::Index().
			 * @param owner Public muxer.
			 * @param encoder Live encoder (must outlive the header).
			 * @return false if owner.Fail() was called.
			 */
			virtual bool ReserveEncoder(StormByte::Multimedia::Pipeline::Muxer& owner,
				StormByte::Multimedia::Pipeline::Encoder& encoder) noexcept = 0;

			/**
			 * @brief Reserves a remux output slot for origin stream @p inIndex.
			 * @param owner Public muxer.
			 * @param inIndex Origin stream index (Remuxer::In).
			 * @return false if owner.Fail() was called.
			 */
			virtual bool ReserveRemux(StormByte::Multimedia::Pipeline::Muxer& owner,
				int inIndex) noexcept = 0;

			/**
			 * @brief Snapshots File attachments for header time.
			 * @param owner Public muxer.
			 * @param file Source file.
			 * @return false if owner.Fail() was called.
			 */
			virtual bool BindAttachments(StormByte::Multimedia::Pipeline::Muxer& owner,
				const File& file) noexcept = 0;

			/**
			 * @brief Queues or writes one packet. Writes the header when ready.
			 * @param owner Public muxer.
			 * @param packet Encoded or remuxed packet.
			 * @return true if the packet was accepted.
			 */
			virtual bool Push(StormByte::Multimedia::Pipeline::Muxer& owner,
				const std::shared_ptr<StormByte::Multimedia::Pipeline::Packet>& packet) noexcept = 0;

			/**
			 * @brief Flushes leftover packets and the trailer.
			 * @param owner Public muxer.
			 */
			virtual void Flush(StormByte::Multimedia::Pipeline::Muxer& owner) noexcept = 0;

			/**
			 * @brief Writes trailer if needed and frees AVIO + context.
			 */
			virtual void Close() noexcept = 0;

		protected:
			/**
			 * @brief Default constructor.
			 */
			Muxer() noexcept = default;

			/**
			 * @brief Move constructor.
			 * @param other Backend to take.
			 */
			Muxer(Muxer&& other) noexcept = default;

			/**
			 * @brief Move assignment.
			 * @param other Backend to take.
			 * @return *this.
			 */
			Muxer& operator=(Muxer&& other) noexcept = default;

			/**
			 * @brief Asks the public Muxer to copy @p encoder into @p avStream.
			 * @param owner Public muxer.
			 * @param encoder Reserved encode lane.
			 * @param avStream libav AVStream*.
			 * @return false if the encoder has no context.
			 */
			bool BindEncoderStream(StormByte::Multimedia::Pipeline::Muxer& owner,
				StormByte::Multimedia::Pipeline::Encoder& encoder, void* avStream) noexcept {
				return owner.BindEncoderStream(encoder, avStream);
			}
	};
}
