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
	class Encoder;

	/**
	 * @enum Status
	 * @brief Lifecycle of a Transcode instance.
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
	 * @class TrackPlan
	 * @brief One mapped track as requested or after the encoder opened.
	 *
	 * Settled fields (sample format, channel counts, frame size) stay
	 * empty until OnSettled.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC TrackPlan {
		public:
			int in = -1;											///< Source stream index
			int out = -1;											///< Destination track index
			Type kind = Type::Video;								///< Video / audio / subtitle
			bool copy = true;										///< Bitstream copy
			const Codec* source = nullptr;							///< Source codec
			const Codec* destination = nullptr;						///< Destination codec, or nullptr if copy
			std::optional<std::string> implementation;				///< Pin (libx265, …)
			std::optional<std::string> language;					///< Language tag
			std::optional<std::string> title;						///< Track title
			std::optional<int> crf;									///< CRF/CQ
			std::optional<std::int64_t> bitRate;					///< Target bitrate
			std::optional<std::int64_t> maxBitRate;					///< VBV ceiling
			std::optional<std::string> preset;						///< Preset
			std::optional<std::string> tune;						///< Tune
			std::map<std::string, std::string> fineTune;			///< Vendor leftovers
			std::optional<int> sampleFormat;						///< Encoder AVSampleFormat
			std::optional<int> sourceChannels;						///< Decoded channel count
			std::optional<int> encoderChannels;						///< Encoder channel count (6 after 7.1 downmix)
			std::optional<int> frameSize;							///< Encoder frame_size
			std::optional<int> sampleRate;							///< Samples per second

			/**
			 * @brief Destructor.
			 */
			virtual ~TrackPlan() noexcept = default;

			/**
			 * @brief Deep copy.
			 * @return New track plan of the same dynamic type.
			 */
			virtual std::unique_ptr<TrackPlan> Clone() const {
				return std::make_unique<TrackPlan>(*this);
			}

			/**
			 * @brief Human-readable line for logs.
			 * @return One or more lines, no trailing newline required.
			 */
			virtual std::string ToString() const;
	};

	/**
	 * @class Plan
	 * @brief Job snapshot. Paid apps derive this and return it from MakePlan().
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Plan {
		public:
			const File* source = nullptr;							///< Opened source
			const Container* container = nullptr;					///< Destination container
			std::filesystem::path destination;						///< Output path
			std::vector<std::unique_ptr<TrackPlan>> tracks;			///< Mapped tracks, out order
			std::vector<int> ignored;								///< Source indexes dropped
			std::size_t videoPacketCeiling = 0;						///< Decode packet queue
			std::size_t muxPacketCeiling = 0;						///< Recode mux queue
			std::size_t copyPacketCeiling = 0;						///< Copy mux queue
			std::size_t videoFrameCeiling = 0;						///< Decoded video frames

			/**
			 * @brief Destructor.
			 */
			virtual ~Plan() noexcept = default;

			/**
			 * @brief Deep copy, including track plans.
			 * @return New plan of the same dynamic type.
			 */
			virtual std::unique_ptr<Plan> Clone() const;

			/**
			 * @brief Human-readable dump.
			 * @return Multi-line text.
			 */
			virtual std::string ToString() const;
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
					 * @param other Source handle.
					 */
					Track(const Track& other) noexcept = default;

					/**
					 * @brief Move constructor.
					 * @param other Source handle.
					 */
					Track(Track&& other) noexcept = default;

					/**
					 * @brief Destructor.
					 */
					~Track() noexcept = default;

					/**
					 * @brief Copy assignment.
					 * @param other Source handle.
					 * @return *this.
					 */
					Track& operator=(const Track& other) noexcept = default;

					/**
					 * @brief Move assignment.
					 * @param other Source handle.
					 * @return *this.
					 */
					Track& operator=(Track&& other) noexcept = default;

					/**
					 * @brief Marks the track as bitstream copy.
					 * @return *this.
					 */
					Track& Copy() noexcept;

					/**
					 * @brief Recodes this track to @p codec.
					 * @param codec Destination registry codec.
					 * @return *this.
					 */
					Track& Codec(const StormByte::Multimedia::Codec& codec) noexcept;

					/**
					 * @brief Pins an FFmpeg encoder implementation name.
					 * @param name avcodec_find_encoder_by_name key.
					 * @return *this.
					 */
					Track& Implementation(std::string name) noexcept;

					/**
					 * @brief Sets CRF/CQ. Clears BitRate.
					 * @param value Quality value.
					 * @return *this.
					 */
					Track& CRF(int value) noexcept;

					/**
					 * @brief Sets target bitrate. Clears CRF.
					 * @param bits_per_second Bits per second.
					 * @return *this.
					 */
					Track& BitRate(std::int64_t bits_per_second) noexcept;

					/**
					 * @brief Sets VBV/max bitrate.
					 * @param bits_per_second Bits per second.
					 * @return *this.
					 */
					Track& MaxBitRate(std::int64_t bits_per_second) noexcept;

					/**
					 * @brief Sets encoder preset.
					 * @param name Preset name.
					 * @return *this.
					 */
					Track& Preset(std::string name) noexcept;

					/**
					 * @brief Sets encoder tune.
					 * @param name Tune name.
					 * @return *this.
					 */
					Track& Tune(std::string name) noexcept;

					/**
					 * @brief Replaces vendor leftovers.
					 * @param options Key/value map.
					 * @return *this.
					 */
					Track& FineTune(std::map<std::string, std::string> options) noexcept;

					/**
					 * @brief Overrides stream language.
					 * @param language ISO tag.
					 * @return *this.
					 */
					Track& Language(std::string language) noexcept;

					/**
					 * @brief Overrides stream title.
					 * @param title Title text.
					 * @return *this.
					 */
					Track& Title(std::string title) noexcept;

				private:
					friend class Transcode;

					/**
					 * @brief Binds this handle to a Slot.
					 * @param owner Parent job.
					 * @param slot Index into m_explicit.
					 */
					Track(Transcode& owner, std::size_t slot) noexcept;

					Transcode* m_owner;								///< Parent job
					std::size_t m_slot;								///< Slot index
			};

			/**
			 * @brief Copy constructor (deleted).
			 */
			Transcode(const Transcode&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source job.
			 */
			Transcode(Transcode&& other) noexcept;

			/**
			 * @brief Destructor. Cancels and joins if still running.
			 */
			virtual ~Transcode() noexcept;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			Transcode& operator=(const Transcode&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source job.
			 * @return *this.
			 */
			Transcode& operator=(Transcode&& other) noexcept;

			/**
			 * @brief Opens a path.
			 * @param logger Required logger.
			 * @param path Source file.
			 * @return Job, or unexpected.
			 */
			static ExpectedTranscode Open(std::shared_ptr<StormByte::Logger::Log> logger,
				const std::filesystem::path& path) noexcept;

			/**
			 * @brief Opens a path with a known duration hint.
			 * @param logger Required logger.
			 * @param path Source file.
			 * @param duration Hint used for Progress().
			 * @return Job, or unexpected.
			 */
			static ExpectedTranscode Open(std::shared_ptr<StormByte::Logger::Log> logger,
				const std::filesystem::path& path, std::chrono::nanoseconds duration) noexcept;

			/**
			 * @brief Opens a consumer buffer.
			 * @param logger Required logger.
			 * @param consumer Source bytes.
			 * @return Job, or unexpected.
			 */
			static ExpectedTranscode Open(std::shared_ptr<StormByte::Logger::Log> logger,
				StormByte::Buffer::Consumer consumer) noexcept;

			/**
			 * @brief Opens a consumer with a known duration hint.
			 * @param logger Required logger.
			 * @param consumer Source bytes.
			 * @param duration Hint used for Progress().
			 * @return Job, or unexpected.
			 */
			static ExpectedTranscode Open(std::shared_ptr<StormByte::Logger::Log> logger,
				StormByte::Buffer::Consumer consumer, std::chrono::nanoseconds duration) noexcept;

			/**
			 * @brief Opened source file.
			 * @return File snapshot.
			 */
			const File& Source() const noexcept;

			/**
			 * @brief Logger bound at Open.
			 * @return Shared logger.
			 */
			const std::shared_ptr<StormByte::Logger::Log>& Logger() const noexcept;

			/**
			 * @brief Maps a video stream.
			 * @param in Source index.
			 * @param out Destination order key.
			 * @return Fluent handle.
			 */
			Track Video(int in, int out) noexcept;

			/**
			 * @brief Maps an audio stream.
			 * @param in Source index.
			 * @param out Destination order key.
			 * @return Fluent handle.
			 */
			Track Audio(int in, int out) noexcept;

			/**
			 * @brief Maps a subtitle stream.
			 * @param in Source index.
			 * @param out Destination order key.
			 * @return Fluent handle.
			 */
			Track Subtitle(int in, int out) noexcept;

			/**
			 * @brief Drops a source stream from the job.
			 * @param in Source index.
			 * @return *this.
			 */
			Transcode& Ignore(int in) noexcept;

			/**
			 * @brief Sets destination container and path.
			 * @param container Writable container.
			 * @param path Output path.
			 * @return *this.
			 */
			Transcode& Destination(const StormByte::Multimedia::Container& container,
				std::filesystem::path path) noexcept;

			/**
			 * @brief Starts workers. Returns immediately.
			 */
			void Run() noexcept;

			/**
			 * @brief Requests abort. Status becomes Aborted after joins.
			 */
			void Cancel() noexcept;

			/**
			 * @brief Pauses workers if Running.
			 */
			void Pause() noexcept;

			/**
			 * @brief Resumes workers if Paused.
			 */
			void Resume() noexcept;

			/**
			 * @brief Current lifecycle.
			 * @return Status value.
			 */
			enum Status Status() const noexcept;

			/**
			 * @brief Whether Status is Error.
			 * @return true after Fail().
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text.
			 * @return Message, or empty.
			 */
			std::optional<std::string> Error() const noexcept;

			/**
			 * @brief Approximate percent, when known.
			 * @return 0..100, or empty before the first update.
			 */
			std::optional<unsigned> Progress() const noexcept;

			/**
			 * @brief true while the job is usable (not Error/Aborted).
			 * @return false after a hard fail or cancel.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @brief Current snapshot (requested map, plus settled fields when known).
			 * @return Plan owned by the caller. Default builds a Plan via MakePlan().
			 */
			virtual std::unique_ptr<Plan> Configuration() const noexcept;

		protected:
			/**
			 * @brief Constructs an empty job. Only Open / derived classes.
			 * @param logger Required logger.
			 * @param file Opened source.
			 */
			Transcode(std::shared_ptr<StormByte::Logger::Log> logger, File file) noexcept;

			/**
			 * @brief Allocates the snapshot type. Paid jobs return a derived Plan.
			 * @return Empty plan of the desired dynamic type.
			 */
			virtual std::unique_ptr<Plan> MakePlan() const noexcept;

			/**
			 * @brief Allocates one track row. Paid jobs return a derived TrackPlan.
			 * @return Empty track plan of the desired dynamic type.
			 */
			virtual std::unique_ptr<TrackPlan> MakeTrackPlan() const noexcept;

			/**
			 * @brief Last chance to raise queue ceilings before workers start.
			 *
			 * Default does nothing.
			 */
			virtual void OnConfigure() noexcept;

			/**
			 * @brief Gate after the map is valid and Destination is set.
			 * @return Running to proceed, Error/Aborted/Stopped to bail.
			 */
			virtual enum Status OnStart() noexcept;

			/**
			 * @brief Requested map, just before workers start.
			 * @param plan Snapshot from Configuration().
			 *
			 * Default logs plan.ToString() at LowLevel.
			 */
			virtual void OnPlan(const Plan& plan) noexcept;

			/**
			 * @brief One recode track finished Encoder::Open.
			 * @param track Settled row (layout, sample format, frame_size).
			 *
			 * Default logs track.ToString() at LowLevel.
			 */
			virtual void OnSettled(const TrackPlan& track) noexcept;

			/**
			 * @brief Progress tick.
			 * @param percent 0..100.
			 *
			 * Default does nothing.
			 */
			virtual void OnProgress(unsigned percent) noexcept;

			/**
			 * @brief Successful flush.
			 *
			 * Default does nothing.
			 */
			virtual void OnDone() noexcept;

			/**
			 * @brief Hard error.
			 * @param message Error text.
			 *
			 * Default does nothing.
			 */
			virtual void OnError(const std::string& message) noexcept;

			/**
			 * @brief Cancel completed.
			 *
			 * Default does nothing.
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
			struct Slot;											///< One mapped or implied track
			class Impl;												///< Queues, worker, flags

			std::shared_ptr<StormByte::Logger::Log> m_logger;		///< Required logger
			std::unique_ptr<File> m_file;							///< Opened source
			std::unique_ptr<Impl> m_impl;							///< Runtime state

			/**
			 * @brief Marks a hard error and cancels workers.
			 * @param reason Message stored in Error().
			 */
			void Fail(std::string reason) noexcept;

			/**
			 * @brief Opens a File and constructs Transcode.
			 * @param logger Required logger.
			 * @param opened File result.
			 * @return Job, or unexpected.
			 */
			static ExpectedTranscode BindLoggerAndFile(std::shared_ptr<StormByte::Logger::Log> logger,
				ExpectedFile opened) noexcept;

			/**
			 * @brief Maps one source stream.
			 * @param in Source index.
			 * @param out Destination order key.
			 * @param kind Expected type.
			 * @return Fluent track handle.
			 */
			Track AddTrack(int in, int out, Type kind) noexcept;

			/**
			 * @brief Worker thread body.
			 */
			void Worker() noexcept;

			/**
			 * @brief Records encoder Open() into the matching Slot and fires OnSettled.
			 * @param in Source stream index of the lane.
			 * @param encoder Encoder that just opened.
			 *
			 * No-op if the encoder is not Opened() or the slot is already settled.
			 */
			void MarkSettled(int in, Encoder& encoder) noexcept;

			/**
			 * @brief Publishes percent to Progress() and OnProgress.
			 * @param percent 0..100.
			 */
			void SetProgress(unsigned percent) noexcept;

			/**
			 * @brief Whether @p slot indexes m_explicit.
			 * @param slot Slot index.
			 * @return true if usable.
			 */
			bool ValidSlot(std::size_t slot) const noexcept;
	};
}
