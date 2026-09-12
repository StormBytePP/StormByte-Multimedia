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

#include <StormByte/buffer/fifo.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/type.hxx>
#include <StormByte/multimedia/visibility.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	class Decoder;	///< Decode backend behind @ref StormByte::Multimedia::Pipeline::Decoder.
	class Demuxer;	///< Demux backend behind @ref StormByte::Multimedia::Pipeline::Demuxer.
}

/**
 * @namespace StormByte::Multimedia
 * @brief Public multimedia types: codecs, containers, streams and files.
 */
namespace StormByte::Multimedia {
	class Origin;
}

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Decoder;
	class Demuxer;
	class Muxer;
	class Packet;
	class Plan;
	class Remuxer;

	/**
	 * @brief Binds one origin track of @p demuxer to @p decoder.
	 * @param demuxer Origin demuxer.
	 * @param decoder Destination decoder.
	 * @return @p decoder.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Decoder& operator>>(Demuxer& demuxer, Decoder& decoder) noexcept;

	/**
	 * @class Demuxer
	 * @brief Reads interleaved compressed packets from the Plan origin.
	 *
	 * Only tracks listed in the bound Plan enter the tube. An origin
	 * stream omitted from Plan::add is never pushed.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Demuxer final: public Step {
		friend class Backend::Pipeline::Demuxer;
		friend class Decoder;
		friend class Muxer;
		friend Decoder& operator>>(Demuxer& demuxer, Decoder& decoder) noexcept;
		friend Demuxer& operator>>(class Plan&& plan, Demuxer& demuxer) noexcept;
		friend Muxer& operator>>(Demuxer& demuxer, Muxer& muxer) noexcept;
		friend Remuxer& operator>>(Demuxer& demuxer, Remuxer& remuxer) noexcept;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Demuxer. Launches; Open waits for a Plan.
			 */
			Demuxer() noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source demuxer.
			 */
			Demuxer(const Demuxer& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Demuxer to take.
			 */
			Demuxer(Demuxer&& other) noexcept = delete;

			/**
			 * @brief Destructor.
			 */
			~Demuxer() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source demuxer.
			 * @return *this.
			 */
			Demuxer& operator=(const Demuxer& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Demuxer to take.
			 * @return *this.
			 */
			Demuxer& operator=(Demuxer&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @brief true if open, not failed and not at EOF.
			 * @return Open and readable.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @name Source
			 * @{
			 */

			/**
			 * @brief Whether the last read hit EOF.
			 * @return true at end of source.
			 */
			bool Eof() const noexcept;

			/**
			 * @brief Presentation time of the last pushed packet.
			 * @return Pts, or empty until a packet with Pts arrives.
			 */
			std::optional<Property::Duration> Position() const noexcept;

			/**
			 * @}
			 */

		private:
			/**
			 * @brief Waits for a Plan, checks it and opens the origin.
			 */
			void Open() noexcept override;

			/**
			 * @brief Reads packets until Fail, stop or EOF.
			 *
			 * Origin streams not listed in the Plan are discarded
			 * and never enter a hopper.
			 */
			void Pump() noexcept override;

			/**
			 * @brief Marks end of source after the last packet.
			 */
			void Finish() noexcept override;

			/**
			 * @brief Marks end of source. Called by the backend on EOF.
			 */
			void ReachedEof() noexcept;

			/**
			 * @brief Origin snapshot owned by the bound Plan.
			 * @return File.
			 */
			const File& OriginFile() const noexcept;

			/**
			 * @brief Path or Consumer held by the origin File.
			 * @return Origin. Valid after plan >> demuxer.
			 */
			Origin& BoundOrigin() noexcept;

			/**
			 * @brief Opens the decode backend for @p decoder.
			 * @param decoder Destination decoder.
			 * @return Backend, or empty after Fail.
			 */
			std::unique_ptr<Backend::Pipeline::Decoder> OpenDecoder(Decoder& decoder) noexcept;

			/**
			 * @brief Builds a public packet. Called from the backend.
			 *
			 * Assigns the next @ref Packet::Serial for @p track and
			 * @ref Packet::Part zero. This is pipe lineage, not a
			 * decoded-frame count.
			 */
			std::shared_ptr<Packet> Wrap(
				int track,
				Type type,
				StormByte::Buffer::FIFO payload,
				std::optional<Property::Duration> pts,
				std::optional<Property::Duration> dts,
				std::optional<Property::Duration> duration,
				bool keyframe) noexcept;

			std::unique_ptr<Backend::Pipeline::Demuxer> m_backend;	///< Format backend
			bool m_eof;												///< End of source
			std::mutex m_planMutex;									///< Guards Plan wait
			std::condition_variable m_planPresent;					///< Woken when a Plan arrives
			std::atomic<std::int64_t> m_positionNs;					///< Last packet Pts, or -1
			std::unordered_map<int, std::uint64_t> m_nextSerial;	///< Next lineage id per origin track
	};
}
