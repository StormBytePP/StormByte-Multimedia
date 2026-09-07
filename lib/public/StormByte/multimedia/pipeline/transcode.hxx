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
#include <StormByte/logger/log.hxx>
#include <StormByte/multimedia/codec.hxx>
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/type.hxx>
#include <StormByte/multimedia/typedefs.hxx>
#include <StormByte/multimedia/visibility.h>

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 */
namespace StormByte::Multimedia::Pipeline {
	/**
	 * @enum Status
	 * @brief Lifecycle of a Transcode instance.
	 * @ingroup multimedia_pipeline
	 */
	enum class Status {
		Stopped,	///< Open succeeded; Run has not started, or OnStart declined
		Running,	///< Workers are alive and not paused
		Paused,		///< Pause(); workers block until Resume or Cancel
		Done,		///< Finished and flushed
		Error,		///< A stage failed; Error() has text
		Aborted		///< Cancel() while Running/Paused, or OnStart returned Aborted
	};

	/**
	 * @class Transcode
	 * @brief High-level job: map tracks, run demux/decode/encode/mux on workers.
	 *
	 * Queue ceilings are protected so a derived job can raise them in
	 * OnConfigure() before workers start. Defaults keep a small working
	 * set; bigger values hide decode jitter at the cost of RAM.
	 *
	 * Recode packets and bitstream-copy packets use separate mux queues
	 * so a high-rate copy track (TrueHD) cannot stall x265.
	 *
	 * Progress is the last monotonic PTS of a muxed video packet over
	 * File::Duration(). Copy tracks do not move the percentage.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Transcode {
		public:
			/**
			 * @class Track
			 * @brief Fluent configuration for one mapped source track.
			 * @ingroup multimedia_pipeline
			 */
			class STORMBYTE_MULTIMEDIA_PUBLIC Track {
				public:
					/**
					 * @brief Copy constructor.
					 */
					Track(const Track& other) noexcept = default;

					/**
					 * @brief Move constructor.
					 */
					Track(Track&& other) noexcept = default;

					/**
					 * @brief Destructor.
					 */
					~Track() noexcept = default;

					/**
					 * @brief Copy assignment.
					 * @return *this.
					 */
					Track& operator=(const Track& other) noexcept = default;

					/**
					 * @brief Move assignment.
					 * @return *this.
					 */
					Track& operator=(Track&& other) noexcept = default;

					/**
					 * @brief Bitstream-copy this track. No decode/encode.
					 * @return *this.
					 */
					Track& Copy() noexcept;

					/**
					 * @brief Recode this track to @p codec.
					 * @param codec Destination StormByte codec (e.g. HEVC).
					 * @return *this.
					 */
					Track& Codec(const StormByte::Multimedia::Codec& codec) noexcept;

					/**
					 * @brief Pin an encoder implementation name (e.g. "libx265").
					 * @param name Encoder name as FFmpeg knows it. Empty clears the pin.
					 * @return *this.
					 */
					Track& Implementation(std::string name) noexcept;

					/**
					 * @brief Constant rate factor.
					 * @param value Encoder CRF.
					 * @return *this.
					 */
					Track& CRF(int value) noexcept;

					/**
					 * @brief Target bitrate.
					 * @param bits_per_second Bits per second.
					 * @return *this.
					 */
					Track& BitRate(std::int64_t bits_per_second) noexcept;

					/**
					 * @brief VBV/HRD maximum bitrate.
					 * @param bits_per_second Bits per second.
					 * @return *this.
					 */
					Track& MaxBitRate(std::int64_t bits_per_second) noexcept;

					/**
					 * @brief Encoder preset name.
					 * @param name Preset string.
					 * @return *this.
					 */
					Track& Preset(std::string name) noexcept;

					/**
					 * @brief Encoder tune name.
					 * @param name Tune string.
					 * @return *this.
					 */
					Track& Tune(std::string name) noexcept;

					/**
					 * @brief Extra encoder key/value pairs (x265-params, etc.).
					 * @param options Map of option name to value.
					 * @return *this.
					 */
					Track& FineTune(std::map<std::string, std::string> options) noexcept;

					/**
					 * @brief Output language tag.
					 * @param language BCP-47 / ISO code.
					 * @return *this.
					 */
					Track& Language(std::string language) noexcept;

					/**
					 * @brief Output track title.
					 * @param title Title metadata.
					 * @return *this.
					 */
					Track& Title(std::string title) noexcept;

				private:
					friend class Transcode;

					/**
					 * @brief Binds this handle to a slot in @p owner.
					 * @param owner Job that owns the slot.
					 * @param slot Index into the explicit map.
					 */
					Track(Transcode& owner, std::size_t slot) noexcept;

					Transcode* m_owner;		///< Owning job
					std::size_t m_slot;		///< Slot index
			};

			Transcode(const Transcode&) = delete;

			/**
			 * @brief Move constructor.
			 */
			Transcode(Transcode&& other) noexcept;

			/**
			 * @brief Joins workers if Run() is still alive.
			 */
			virtual ~Transcode() noexcept;

			Transcode& operator=(const Transcode&) = delete;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			Transcode& operator=(Transcode&& other) noexcept;

			/**
			 * @brief Opens a file path. Duration comes from the container.
			 * @param logger Required logger (ThreadedLog recommended).
			 * @param path Source path.
			 * @return Job, or an error.
			 */
			static ExpectedTranscode Open(std::shared_ptr<StormByte::Logger::Log> logger,
				const std::filesystem::path& path) noexcept;

			/**
			 * @brief Opens a file path and overrides advertised duration.
			 * @param logger Required logger.
			 * @param path Source path.
			 * @param duration Duration used for Progress().
			 * @return Job, or an error.
			 */
			static ExpectedTranscode Open(std::shared_ptr<StormByte::Logger::Log> logger,
				const std::filesystem::path& path, std::chrono::nanoseconds duration) noexcept;

			/**
			 * @brief Opens a consumer buffer. Duration comes from the container.
			 * @param logger Required logger.
			 * @param consumer Source bytes.
			 * @return Job, or an error.
			 */
			static ExpectedTranscode Open(std::shared_ptr<StormByte::Logger::Log> logger,
				StormByte::Buffer::Consumer consumer) noexcept;

			/**
			 * @brief Opens a consumer buffer and overrides advertised duration.
			 * @param logger Required logger.
			 * @param consumer Source bytes.
			 * @param duration Duration used for Progress().
			 * @return Job, or an error.
			 */
			static ExpectedTranscode Open(std::shared_ptr<StormByte::Logger::Log> logger,
				StormByte::Buffer::Consumer consumer, std::chrono::nanoseconds duration) noexcept;

			/**
			 * @brief Source file opened by Open().
			 * @return Source file.
			 */
			const File& Source() const noexcept;

			/**
			 * @brief Logger passed to Open().
			 * @return Logger.
			 */
			const std::shared_ptr<StormByte::Logger::Log>& Logger() const noexcept;

			/**
			 * @brief Maps a video source stream to an output index.
			 * @param in Source stream index.
			 * @param out Output index (compacted by Destination).
			 * @return Track handle for Copy() or Codec().
			 */
			Track Video(int in, int out) noexcept;

			/**
			 * @brief Maps an audio source stream to an output index.
			 * @param in Source stream index.
			 * @param out Output index.
			 * @return Track handle.
			 */
			Track Audio(int in, int out) noexcept;

			/**
			 * @brief Maps a subtitle source stream to an output index.
			 * @param in Source stream index.
			 * @param out Output index.
			 * @return Track handle.
			 */
			Track Subtitle(int in, int out) noexcept;

			/**
			 * @brief Drops a source stream from the job.
			 * @param in Source stream index.
			 * @return *this.
			 */
			Transcode& Ignore(int in) noexcept;

			/**
			 * @brief Sets the output container and path. Compacts mapped tracks.
			 * @param container Destination container.
			 * @param path Destination path.
			 * @return *this.
			 */
			Transcode& Destination(const StormByte::Multimedia::Container& container,
				std::filesystem::path path) noexcept;

			/**
			 * @brief Starts workers and returns immediately.
			 */
			void Run() noexcept;

			/**
			 * @brief Requests abort. Status becomes Aborted after joins.
			 */
			void Cancel() noexcept;

			/**
			 * @brief Pauses workers between units of work. x265 may finish the in-flight frame.
			 */
			void Pause() noexcept;

			/**
			 * @brief Resumes after Pause().
			 */
			void Resume() noexcept;

			/**
			 * @brief Current lifecycle value.
			 * @return Status.
			 */
			enum Status Status() const noexcept;

			/**
			 * @brief Whether Status() is Error.
			 * @return true on Error.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text when Failed().
			 * @return Message, or empty.
			 */
			std::optional<std::string> Error() const noexcept;

			/**
			 * @brief Muxed video progress in percent, if a duration is known.
			 * @return 0–100, or empty when Stopped / no duration.
			 */
			std::optional<unsigned> Progress() const noexcept;

			/**
			 * @brief Whether the job is usable (not Error).
			 */
			explicit operator bool() const noexcept;

		protected:
			/**
			 * @brief Constructs an empty job. Only Open / derived classes.
			 * @param logger Logger retained for the job lifetime.
			 * @param file Opened source.
			 */
			Transcode(std::shared_ptr<StormByte::Logger::Log> logger, File file) noexcept;

			/**
			 * @brief Called on the worker thread before stages open.
			 * Raise queue ceilings here. Default does nothing.
			 */
			virtual void OnConfigure() noexcept;

			/**
			 * @brief Called after the map is complete, before threads start.
			 * @return Running to proceed, Stopped to skip, Aborted or Error to bail.
			 */
			virtual enum Status OnStart() noexcept;

			/**
			 * @brief Called when Progress() changes, including the initial 0.
			 * @param percent New percent.
			 */
			virtual void OnProgress(unsigned percent) noexcept;

			/**
			 * @brief Called after a successful flush when Status becomes Done.
			 */
			virtual void OnDone() noexcept;

			/**
			 * @brief Called when Status becomes Error.
			 * @param message Error text.
			 */
			virtual void OnError(const std::string& message) noexcept;

			/**
			 * @brief Called when Status becomes Aborted.
			 */
			virtual void OnAborted() noexcept;

			/**
			 * @brief Compressed packets waiting to be decoded (video recode).
			 * Default 16. Raise in OnConfigure() if decode bursts starve x265.
			 */
			std::size_t m_videoPacketCeiling = 16;

			/**
			 * @brief Compressed packets waiting to be decoded (audio/subtitle recode).
			 * Default 32.
			 */
			std::size_t m_packetCeiling = 32;

			/**
			 * @brief Encoded recode packets waiting for the mux thread.
			 * Default 64. Not used for bitstream copy.
			 */
			std::size_t m_muxPacketCeiling = 64;

			/**
			 * @brief Bitstream-copy packets waiting for the mux thread.
			 * Default 8192. TrueHD emits thousands of small packets; a
			 * shared mux queue would block demux and starve video.
			 */
			std::size_t m_copyPacketCeiling = 8192;

			/**
			 * @brief Decoded video frames waiting for the encoder thread.
			 * Default 8. Each 4K 10-bit frame is tens of MiB.
			 */
			std::size_t m_videoFrameCeiling = 8;

		private:
			struct Slot;
			class Impl;

			std::shared_ptr<StormByte::Logger::Log> m_logger;	///< Job logger
			std::unique_ptr<File> m_file;						///< Source file
			std::unique_ptr<Impl> m_impl;						///< Queues, status, worker

			/**
			 * @brief Marks the job Error and wakes workers.
			 * @param reason Message stored in Error().
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @brief Shared Open() tail: logger + opened file.
			 * @param logger Required logger.
			 * @param opened File::Open result.
			 * @return Job, or the file error.
			 */
			static ExpectedTranscode BindLoggerAndFile(std::shared_ptr<StormByte::Logger::Log> logger,
				ExpectedFile opened) noexcept;

			/**
			 * @brief Inserts or updates an explicit map slot.
			 * @param in Source index.
			 * @param out Output key.
			 * @param kind Video, Audio or Subtitle.
			 * @return Track handle.
			 */
			Track AddTrack(int in, int out, Type kind) noexcept;

			/**
			 * @brief Worker thread body.
			 */
			void Worker() noexcept;

			/**
			 * @brief Stores a monotonic percent and calls OnProgress().
			 * @param percent Value clamped to 0–100.
			 */
			void SetProgress(unsigned percent) noexcept;

			/**
			 * @brief Whether @p slot is an explicit map entry.
			 * @param slot Slot index.
			 * @return true if usable.
			 */
			bool ValidSlot(std::size_t slot) const noexcept;
	};
}
