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

#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/filters/pipe.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/visibility.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 */
namespace StormByte::Multimedia::Pipeline {
	class Copy;
	class Demux;
	class Mux;

	/**
	 * @defgroup mux_ops Mux stream operators
	 * @{
	 */

	/**
	 * @brief Binds @p encoder's Index() as a mux track. Never throws.
	 * @param encoder Source encoder (must outlive the mux until the header).
	 * @param mux Destination.
	 * @return @p encoder.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Encoder& operator>>(Encoder& encoder, Mux& mux) noexcept;

	/**
	 * @brief Reserves Copy::Index() on @p mux as a remux track. Never throws.
	 *
	 * Declared in copy.hxx. The track is ready immediately.
	 * @param copy Bound copy track (must outlive the mux until the header).
	 * @param mux Destination.
	 * @return @p copy.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Copy& operator>>(Copy& copy, Mux& mux) noexcept;

	/**
	 * @brief Opens @p mux on @p path using the constructor container. Never throws.
	 * @param mux Muxer.
	 * @param path Destination file.
	 * @return @p mux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Mux& operator>>(Mux& mux, const std::filesystem::path& path) noexcept;

	/**
	 * @brief Writes @p packet after the packet pipe. Never throws.
	 *
	 * Encoded packets use Encoder::Index(). Copied packets keep the demux
	 * stream index; the mux remaps them to the reserved output slot.
	 * @param packet Encoded or copied packet.
	 * @param mux Destination.
	 * @return @p packet.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Packet& operator>>(Packet& packet, Mux& mux) noexcept;

	/**
	 * @brief Snapshots File::Attachments() onto @p mux. Never throws.
	 *
	 * Must run before the header. If the destination container has no
	 * Operation::Attach and the file has attachments, the mux fails.
	 * Attachments are written as real AttachedFile streams, never as
	 * AV_DISPOSITION_ATTACHED_PIC.
	 * @param file Opened source file.
	 * @param mux Destination.
	 * @return @p mux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Mux& operator>>(const File& file, Mux& mux) noexcept;

	/**
	 * @brief Forwards the demuxer's File attachments onto @p mux. Never throws.
	 *
	 * Equivalent to `*demux.file >> mux` when the demuxer was opened with
	 * `file >> demux`. No-op if the demuxer has no File pointer.
	 * @param demux Open demuxer bound to a File.
	 * @param mux Destination.
	 * @return @p mux.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Mux& operator>>(Demux& demux, Mux& mux) noexcept;

	/** @} */

	/**
	 * @class Mux
	 * @brief Writes interleaved compressed packets to a destination file.
	 *
	 * The destination container is fixed at construction. mux >> path only
	 * opens AVIO. encoder >> mux and copy >> mux reserve Index().
	 * file >> mux (or demux >> mux) binds source attachments; they are
	 * written at header time as container-native attachments.
	 * packet >> mux queues until every reserved encoder has opened
	 * (bound copies are already ready), then writes the header and drains.
	 * Flush() signals EOF on every reserved encoder, writes leftover
	 * packets and the trailer. The destructor calls Flush() if needed.
	 * Format metadata ENCODER is StormByte-Multimedia plus the build version.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Mux {
		public:
			/**
			 * @name Construction
			 * @{
			 */

			/**
			 * @brief Muxer for @p container. Does not open a file.
			 * @param container Registry container. Must HasAccess(Write).
			 */
			explicit Mux(const Container& container) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			Mux(const Mux&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Muxer to take.
			 */
			Mux(Mux&& other) noexcept;

			/**
			 * @brief Destructor. Flushes encoders and writes the trailer.
			 */
			~Mux() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Mux& operator=(const Mux&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Muxer to take.
			 * @return *this.
			 */
			Mux& operator=(Mux&& other) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name State
			 * @{
			 */

			/**
			 * @brief true if a destination is bound and not failed.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @brief Destination container bound at construction.
			 * @return Registry container.
			 */
			const Container& Destination() const noexcept;

			/**
			 * @brief Whether a hard error occurred.
			 * @return true on open/write error.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text, if Failed().
			 * @return Message, or empty.
			 */
			const std::optional<std::string>& Error() const noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Filters
			 * @{
			 */

			/**
			 * @brief Packet filter pipe. Add steps before the first packet write.
			 * @return Pipe.
			 */
			Filter::Pipe& Pipe() noexcept;

			/**
			 * @brief Packet filter pipe.
			 * @return Pipe.
			 */
			const Filter::Pipe& Pipe() const noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Finish
			 * @{
			 */

			/**
			 * @brief Flushes every reserved encoder and writes leftover packets.
			 *
			 * Calls Encoder::Flush() on each encode track, writes queued
			 * packets, then the container trailer. Safe to call more than
			 * once. The destructor calls this automatically; keep the
			 * encoders alive until after Flush() (declare Mux last, or call
			 * Flush() yourself before the encoders die).
			 */
			void Flush() noexcept;

			/**
			 * @}
			 */

			friend Encoder& operator>>(Encoder& encoder, Mux& mux) noexcept;
			friend Copy& operator>>(Copy& copy, Mux& mux) noexcept;
			friend Mux& operator>>(Mux& mux, const std::filesystem::path& path) noexcept;
			friend Packet& operator>>(Packet& packet, Mux& mux) noexcept;
			friend Mux& operator>>(const File& file, Mux& mux) noexcept;
			friend Mux& operator>>(Demux& demux, Mux& mux) noexcept;
			friend class Copy;

		private:
			class Impl;

			const Container* m_container;				///< Destination container
			std::unique_ptr<Impl> m_impl;				///< Output format context
			Filter::Pipe m_pipe;						///< Packet steps
			bool m_failed;								///< Hard error
			std::optional<std::string> m_error;			///< Failure text

			/**
			 * @brief Marks a hard error and drops the backend.
			 * @param reason Message.
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @brief Writes header when every reserved encoder is open.
			 *
			 * Bound copy tracks are ready as soon as they are reserved.
			 * Source attachments are written just before avformat_write_header.
			 * Stamps format metadata ENCODER as StormByte-Multimedia <version>
			 * so FFmpeg does not write Lavf*.
			 * @return false if Fail() was called.
			 */
			bool WriteHeaderIfReady() noexcept;

			/**
			 * @brief Writes one packet after the header.
			 * @param packet Source packet (output index after remap).
			 * @return false if Fail() was called.
			 */
			bool WritePacket(Packet& packet) noexcept;

			/**
			 * @brief Adds File attachments as AVMEDIA_TYPE_ATTACHMENT streams.
			 *
			 * Uses extradata + filename/mimetype metadata. Never sets
			 * AV_DISPOSITION_ATTACHED_PIC.
			 * @return false if Fail() was called.
			 */
			bool WriteAttachments() noexcept;

			/**
			 * @brief Flush + close. Used from the destructor.
			 */
			void Finish() noexcept;
	};
}
