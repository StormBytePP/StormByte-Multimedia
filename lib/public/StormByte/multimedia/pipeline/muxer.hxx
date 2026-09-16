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

#include <StormByte/logger/log.hxx>
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/step.hxx>
#include <StormByte/multimedia/property/duration.hxx>
#include <StormByte/multimedia/visibility.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline
 * @brief Multimedia-owned pipeline stages and unit holders.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline {
	class Muxer;	///< Mux backend behind @ref StormByte::Multimedia::Pipeline::Muxer.
}

namespace StormByte::Multimedia::Backend::Pipeline::Detail::Worker {
	class Mux;
}

/**
 * @namespace StormByte::Multimedia::Backend::Pipeline::Detail::Muxer::Matroska
 * @brief Matroska mux leaf.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Backend::Pipeline::Detail::Muxer::Matroska {
	class Container;	///< Matroska mux backend.
}

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Demuxer;
	class Encoder;
	class Muxer;
	class Remuxer;

	/**
	 * @brief Reserves @p encoder as an output track of @p muxer.
	 * @param encoder Live encoder.
	 * @param muxer Destination.
	 * @return @p encoder.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Encoder& operator>>(Encoder& encoder, Muxer& muxer) noexcept;

	/**
	 * @brief Reserves @p remuxer as an output track of @p muxer.
	 * @param remuxer Live remuxer.
	 * @param muxer Destination.
	 * @return @p remuxer.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Remuxer& operator>>(Remuxer& remuxer, Muxer& muxer) noexcept;

	/**
	 * @brief Binds the output path of @p muxer.
	 * @param muxer Muxer.
	 * @param path Destination file.
	 * @return @p muxer.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Muxer& operator>>(Muxer& muxer, const std::filesystem::path& path) noexcept;

	/**
	 * @brief Snapshots attachments of @p file onto @p muxer.
	 * @param file Source file.
	 * @param muxer Destination.
	 * @return @p muxer.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Muxer& operator>>(const File& file, Muxer& muxer) noexcept;

	/**
	 * @brief Binds @p demuxer as remux origin and forwards its attachments.
	 *
	 * Required when any remux track is reserved. Without it the muxer
	 * cannot clone origin codec parameters.
	 *
	 * @param demuxer Origin demuxer. Must outlive header write.
	 * @param muxer Destination.
	 * @return @p muxer.
	 */
	STORMBYTE_MULTIMEDIA_PUBLIC Muxer& operator>>(Demuxer& demuxer, Muxer& muxer) noexcept;

	/**
	 * @class Muxer
	 * @brief Writes interleaved packets to a destination container.
	 *
	 * Notice: destination path when bound, closed once.
	 * LowLevel writes always; the shared logger throttles.
	 * Remux and encode lanes share this worker.
	 *
	 * Does not write the container header until @ref Armed is true.
	 * @ref Ready is Status Ready and @ref Armed.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Muxer final: public Step {
		friend class Backend::Pipeline::Detail::Muxer::Matroska::Container;
		friend class Backend::Pipeline::Muxer;
		friend class Backend::Pipeline::Detail::Worker::Mux;
		friend Encoder& operator>>(Encoder& encoder, Muxer& muxer) noexcept;
		friend Muxer& operator>>(const File& file, Muxer& muxer) noexcept;
		friend Muxer& operator>>(Demuxer& demuxer, Muxer& muxer) noexcept;
		friend Muxer& operator>>(Muxer& muxer, const std::filesystem::path& path) noexcept;
		friend Remuxer& operator>>(Remuxer& remuxer, Muxer& muxer) noexcept;

		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Muxer for @p container. Destination path is bound later.
			 * @param log Shared logger. Empty pointer means no log.
			 * @param container Writable registry container.
			 */
			explicit Muxer(std::shared_ptr<StormByte::Logger::Log> log,
				const Container& container) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source muxer.
			 */
			Muxer(const Muxer& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Muxer to take.
			 */
			Muxer(Muxer&& other) noexcept = delete;

			/**
			 * @brief Destructor. @ref Step::Join Halt s before backends die.
			 */
			~Muxer() noexcept override;

			/**
			 * @brief Copy assignment.
			 * @param other Source muxer.
			 * @return *this.
			 */
			Muxer& operator=(const Muxer& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Muxer to take.
			 * @return *this.
			 */
			Muxer& operator=(Muxer&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @brief true if the destination is bound and the muxer has not failed.
			 * @return Open and writable.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @brief true after Finish flushed the trailer, or after Fail.
			 * @return Muxer will not accept more packets.
			 */
			bool Closed() const noexcept;

			/**
			 * @brief Presentation time of the last packet written.
			 * @return Pts of the last video packet, or of audio if no video
			 *         has been written yet.
			 */
			std::optional<Property::Duration> Position() const noexcept;

			/**
			 * @brief Destination container.
			 * @return Registry container passed to the constructor.
			 */
			const Container& Destination() const noexcept;

			/**
			 * @brief Whether every muxable Plan track has a reserved hopper.
			 *
			 * Counts Video, Audio and Subtitle entries in @ref Plan::Tracks.
			 * Attachments are written at header time and are not hoppers.
			 * true when reserved slots equal that count. false with no Plan,
			 * or while @c operator>> is still running.
			 *
			 * Does not Fail. The owner of the graph (@ref Filters::Close
			 * or Transcoder) checks this after wiring and Fails the Muxer
			 * if it is still false.
			 *
			 * @return Arming state of the output graph.
			 */
			bool Armed() const noexcept;

			/**
			 * @brief Open finished without Fail or Stop, and @ref Armed.
			 * @return Step Status is Ready and every muxable Plan track is reserved.
			 */
			bool Ready() const noexcept override;

			/**
			 * @brief Ceiling of the muxer input hopper.
			 * @return Max queued packets. Never 0.
			 */
			std::size_t InputCeiling() const noexcept override {
				return Ceiling;
			}

			/**
			 * @name Stream tags
			 * @{
			 */

			/**
			 * @brief Language tag for mux output @p output_index.
			 * @param output_index Mux destination order key.
			 * @return Tag, or empty.
			 */
			std::optional<std::string> Language(int output_index) const noexcept;

			/**
			 * @brief Sets the language tag for mux output @p output_index.
			 * @param output_index Mux destination order key.
			 * @param language BCP-47 / ISO tag. Empty clears.
			 */
			void Language(int output_index, std::string language) noexcept;

			/**
			 * @brief Title tag for mux output @p output_index.
			 * @param output_index Mux destination order key.
			 * @return Title, or empty.
			 */
			std::optional<std::string> Title(int output_index) const noexcept;

			/**
			 * @brief Sets the title tag for mux output @p output_index.
			 * @param output_index Mux destination order key.
			 * @param title Stream title. Empty clears.
			 */
			void Title(int output_index, std::string title) noexcept;

			/**
			 * @}
			 */

		private:
			using Step::Log;

			/**
			 * @brief Blocks until Armed, Failed or Stopping.
			 */
			void WaitArmed() noexcept;

			/**
			 * @brief Copies an opened encoder into a libav output stream.
			 * @param encoder Reserved encode lane.
			 * @param avStream libav AVStream*.
			 * @return false if the encoder has no context.
			 */
			bool BindEncoderStream(Encoder& encoder, void* avStream) noexcept;

			/**
			 * @brief Clones remux codecpar from the bound demuxer.
			 * @param inIndex Origin stream index.
			 * @param params Owned AVCodecParameters* on success.
			 * @param timeBase AVRational*.
			 * @return false if the origin is not ready.
			 */
			bool RemuxCodec(int inIndex, void*& params, void* timeBase) noexcept;

			/**
			 * @brief Video / Audio / Subtitle entries in the bound Plan.
			 * @return 0 when there is no Plan.
			 */
			std::size_t ExpectedSlots() const noexcept;

			static constexpr std::size_t Ceiling = 64;							///< Input hopper ceiling
			const Container* m_container;									///< Destination container
			std::unique_ptr<Backend::Pipeline::Muxer> m_backend;				///< Format backend
			Demuxer* m_origin;												///< Set only by demuxer >> muxer. Not owned
			std::map<int, std::string> m_language;							///< Per-output language
			std::map<int, std::string> m_title;								///< Per-output title
			std::atomic<bool> m_closed;										///< Set by Finish / Fail
			std::atomic<std::size_t> m_reserved;
			std::atomic<std::int64_t> m_positionNs;							///< Last written Pts, or -1
			Join m_join{*this};												///< Halt before other members die
	};
}
