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
#include <StormByte/clonable.hxx>
#include <StormByte/logger/log.hxx>
#include <StormByte/multimedia/codec.hxx>
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/plan.hxx>
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
#include <type_traits>
#include <utility>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Pipeline
 * @brief Demux / decode / filter / encode / mux types.
 *
 * @ingroup multimedia_pipeline
 */
namespace StormByte::Multimedia::Pipeline {
	class Encoder;

	/**
	 * @namespace Engine
	 * @brief Private backends. Public headers only forward-declare them.
	 *
	 * @ingroup multimedia_pipeline
	 */
	namespace Engine {
		/**
		 * @namespace Transcode
		 * @brief Job map and coordinator. Not a tube Step.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Transcode {
			class Engine;
		}
	}

	/**
	 * @enum Status
	 * @brief Lifecycle of a @ref Transcode instance.
	 *
	 * @ref Open leaves the job @ref Status::Stopped. @ref Run starts
	 * the coordinator. @ref OnStart may return Stopped / Error /
	 * Aborted to bail without starting the job.
	 *
	 * @ingroup multimedia_pipeline
	 */
	enum class Status {
		Stopped,	///< Open succeeded; Run has not started, or OnStart declined
		Running,	///< Coordinator is alive and not paused
		Paused,		///< Pause(); coordinator blocks until Resume or Cancel
		Done,		///< Finished and flushed
		Error,		///< A stage failed; Error() has text
		Aborted		///< Cancel() while Running/Paused, or OnStart returned Aborted
	};

	/**
	 * @class TrackSettled
	 * @brief What an encoder actually opened. Not intention.
	 *
	 * @ref Plan / @ref Track are the request. This type is filled
	 * after @ref Encoder is open and passed to
	 * @ref Transcode::OnSettled. Derive it and return the type from
	 * @ref Transcode::EmptySettled to carry extra fields.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC TrackSettled:
		public StormByte::Clonable<TrackSettled, std::unique_ptr<TrackSettled>> {
		public:
			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Empty settled row.
			 */
			TrackSettled() noexcept = default;

			/**
			 * @brief Copy constructor.
			 * @param other Source row.
			 */
			TrackSettled(const TrackSettled& other) = default;

			/**
			 * @brief Move constructor.
			 * @param other Row to take.
			 */
			TrackSettled(TrackSettled&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			virtual ~TrackSettled() noexcept override = default;

			/**
			 * @brief Copy assignment.
			 * @param other Source row.
			 * @return *this.
			 */
			TrackSettled& operator=(const TrackSettled& other) = default;

			/**
			 * @brief Move assignment.
			 * @param other Row to take.
			 * @return *this.
			 */
			TrackSettled& operator=(TrackSettled&& other) noexcept = default;

			/**
			 * @}
			 */

			/**
			 * @brief Deep copy.
			 * @return Owning pointer to a new row of the same dynamic type.
			 */
			inline PointerType Clone() const override {
				return MakePointer<TrackSettled>(*this);
			}

			/**
			 * @brief Move into a new pointer.
			 * @return Owning pointer to the moved row.
			 */
			inline PointerType Move() override {
				return MakePointer<TrackSettled>(std::move(*this));
			}

			/**
			 * @brief Human-readable line for logs.
			 * @return One or more lines, no trailing newline required.
			 */
			virtual std::string ToString() const;

			int In = -1;												///< Origin stream index
			int Out = -1;												///< Mux slot (order in @ref Plan::Tracks)
			Type Kind = Type::Unknown;									///< Media kind
			const Codec* Source = nullptr;								///< Origin codec
			const Codec* Destination = nullptr;							///< Opened encoder codec, or nullptr if remux
			std::optional<std::string> Implementation;					///< Opened encoder pin
			std::optional<int> Crf;										///< CRF/CQ actually used
			std::optional<std::int64_t> BitRate;						///< Bitrate actually used
			std::optional<std::int64_t> MaxBitRate;						///< VBV actually used
			std::optional<std::string> Preset;							///< Preset actually used
			std::optional<std::string> Tune;							///< Tune actually used
			std::map<std::string, std::string> FineTune;				///< Vendor leftovers actually used
			std::optional<int> SampleFormat;							///< Encoder AVSampleFormat
			std::optional<int> SourceChannels;							///< Decoded channel count
			std::optional<int> EncoderChannels;							///< Encoder channel count
			std::optional<int> FrameSize;								///< Encoder frame_size
			std::optional<int> SampleRate;								///< Samples per second
	};

	/**
	 * @class Transcode
	 * @brief Facade that maps tracks and runs one file-to-file job.
	 *
	 * Connects @c operator>> and @ref Route for you. The stock class
	 * is a complete job: open a @ref File, choose origin streams,
	 * remux or encode each one, write another file. You do not have
	 * to derive anything to transcode.
	 *
	 * It is also the extension point. Nothing in the extra surface
	 * is required. Mix what you need:
	 *
	 * - Hooks only. Keep the stock @ref Plan and override
	 *   @ref OnConfigure, @ref OnStart, @ref OnPlan,
	 *   @ref OnSettled, @ref OnProgress, @ref OnDone,
	 *   @ref OnError, @ref OnAborted.
	 * - A richer intention. Override @ref EmptyPlan and return a
	 *   type derived from @ref Plan. @ref Plan::Check is virtual
	 *   on that type. You may still use the stock
	 *   @ref TrackSettled.
	 * - A richer settled row. Override @ref EmptySettled and return
	 *   a type derived from @ref TrackSettled. You may still use
	 *   the stock @ref Plan.
	 * - Both. Derived @ref Plan plus derived @ref TrackSettled plus
	 *   the hooks you care about.
	 *
	 * @ref EmptyPlan / @ref EmptySettled only pick the dynamic type.
	 * This class still fills tracks from the fluent map and settled
	 * fields from the opened @ref Encoder. One instance is one
	 * source and one destination; another job is another instance.
	 *
	 * @ingroup multimedia_pipeline
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC Transcode {
		public:
			/**
			 * @class Track
			 * @brief Fluent handle for one origin stream in this job.
			 *
			 * Writes the matching @ref Config leaf. Mux order is the
			 * @c out passed to @ref Video / @ref Audio / @ref Subtitle,
			 * not a field here. Call before @ref Run.
			 *
			 * @ingroup multimedia_pipeline
			 */
			class STORMBYTE_MULTIMEDIA_PUBLIC Track {
				public:
					/**
					 * @name Lifecycle
					 * @{
					 */

					/**
					 * @brief Copy constructor.
					 * @param other Source handle.
					 */
					Track(const Track& other) noexcept = default;

					/**
					 * @brief Move constructor.
					 * @param other Handle to take.
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
					 * @param other Handle to take.
					 * @return *this.
					 */
					Track& operator=(Track&& other) noexcept = default;

					/**
					 * @}
					 */

					/**
					 * @brief Remux this track (no destination codec).
					 * @return *this.
					 */
					Track& Remux() noexcept;

					/**
					 * @brief Encodes this track to @p codec.
					 * @param codec Destination registry codec.
					 * @return *this.
					 */
					Track& Codec(const StormByte::Multimedia::Codec& codec) noexcept;

					/**
					 * @brief Pins an FFmpeg encoder name.
					 * @param name Table name.
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
					 * @brief Sets VBV / max bitrate.
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
					 * @param options Key / value map.
					 * @return *this.
					 */
					Track& FineTune(std::map<std::string, std::string> options) noexcept;

					/**
					 * @brief Overrides stream language.
					 * @param language ISO tag. Empty clears the override.
					 * @return *this.
					 */
					Track& Language(std::string language) noexcept;

					/**
					 * @brief Overrides stream title.
					 * @param title Title text. Empty clears the override.
					 * @return *this.
					 */
					Track& Title(std::string title) noexcept;

					/**
					 * @brief Appends a Process or Packet filter to this track.
					 * @tparam FilterType Child of @ref Filter::Process or
					 *         @ref Filter::Packet. Not @ref Filter::Analytics.
					 * @param args Constructor arguments, forwarded.
					 * @return *this.
					 *
					 * Call before @ref Transcode::Run. Frame filters on a
					 * remux track are dropped by @ref Route. Analytics attach
					 * on @ref Transcode::Filter.
					 */
					template<typename FilterType, typename... Args>
					Track& Filter(Args&&... args) noexcept {
						static_assert(!std::is_base_of_v<Filter::Analytics, FilterType>,
							"Analytics attach on Transcode::Filter, not Track::Filter");
						m_owner->AttachFilter(m_slot,
							std::make_shared<FilterType>(std::forward<Args>(args)...));
						return *this;
					}

				private:
					friend class Transcode;

					/**
					 * @brief Binds this handle to a mapped slot.
					 * @param owner Parent job.
					 * @param slot Index into the engine map.
					 */
					Track(Transcode& owner, std::size_t slot) noexcept;

					Transcode* m_owner;									///< Parent job
					std::size_t m_slot;									///< Engine map index
			};

			/**
			 * @name Lifecycle
			 * @{
			 */

			/**
			 * @brief Constructs an empty job. Only Open / derived classes.
			 * @param logger Required logger.
			 * @param file Opened source (moved).
			 */
			Transcode(std::shared_ptr<StormByte::Logger::Log> logger, File&& file) noexcept;

			/**
			 * @brief Copy constructor.
			 * @param other Source job.
			 */
			Transcode(const Transcode& other) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Job to take.
			 */
			Transcode(Transcode&& other) noexcept = delete;

			/**
			 * @brief Destructor. Stops the coordinator and joins.
			 */
			virtual ~Transcode() noexcept;

			/**
			 * @brief Copy assignment.
			 * @param other Source job.
			 * @return *this.
			 */
			Transcode& operator=(const Transcode& other) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Job to take.
			 * @return *this.
			 */
			Transcode& operator=(Transcode&& other) noexcept = delete;

			/**
			 * @}
			 */

			/**
			 * @name Open
			 * @{
			 */

			/**
			 * @brief Opens @p source and binds @p destination.
			 * @param logger Required logger.
			 * @param source Input path.
			 * @param destination Output path.
			 * @param duration Authoritative container duration, if known.
			 *        Empty runs the normal probe. A value skips the packet scan.
			 * @return Job, or unexpected.
			 *
			 * Shorthand for @ref File::Open plus the public constructor.
			 * The destination container is still set with @ref Destination
			 * before @ref Run.
			 */
			static ExpectedTranscode Open(std::shared_ptr<StormByte::Logger::Log> logger,
				const std::filesystem::path& source,
				const std::filesystem::path& destination,
				std::optional<std::chrono::nanoseconds> duration = std::nullopt) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Source
			 * @{
			 */

			/**
			 * @brief Opened source file.
			 * @return File snapshot.
			 *
			 * After @ref Run hands the File to the Plan, this is
			 * @c Plan::Source().
			 */
			const File& Source() const noexcept;

			/**
			 * @brief Logger bound at Open.
			 * @return Logger.
			 */
			const std::shared_ptr<StormByte::Logger::Log>& Logger() const noexcept;

			/**
			 * @brief Intention built for this job, if any.
			 * @return Plan, or empty before Destination / Run.
			 */
			inline const std::shared_ptr<class Plan>& Plan() const noexcept {
				return m_plan;
			}

			/**
			 * @}
			 */

			/**
			 * @name Map
			 * @{
			 */

			/**
			 * @brief Maps a video origin stream.
			 * @param in Origin stream index.
			 * @param out Mux slot.
			 * @return Fluent handle.
			 */
			Track Video(int in, int out) noexcept;

			/**
			 * @brief Maps an audio origin stream.
			 * @param in Origin stream index.
			 * @param out Mux slot.
			 * @return Fluent handle.
			 */
			Track Audio(int in, int out) noexcept;

			/**
			 * @brief Maps a subtitle origin stream.
			 * @param in Origin stream index.
			 * @param out Mux slot.
			 * @return Fluent handle.
			 */
			Track Subtitle(int in, int out) noexcept;

			/**
			 * @brief Drops an origin stream (omit from the Plan).
			 * @param in Origin stream index.
			 * @return *this.
			 */
			Transcode& Ignore(int in) noexcept;

			/**
			 * @brief Appends an analytics filter to every encode lane.
			 * @tparam FilterType Child of @ref Filter::Analytics.
			 * @param args Constructor arguments, forwarded.
			 * @return *this.
			 */
			template<typename FilterType, typename... Args>
			Transcode& Filter(Args&&... args) noexcept {
				static_assert(std::is_base_of_v<Filter::Analytics, FilterType>,
					"Track filters attach on Track::Filter");
				AttachAnalytics(std::make_shared<FilterType>(std::forward<Args>(args)...));
				return *this;
			}

			/**
			 * @brief Sets the destination container and path.
			 * @param container Registry destination container.
			 * @param path Output path.
			 * @return *this.
			 *
			 * Required before @ref Run. Closes the job identity together
			 * with the File from Open.
			 */
			Transcode& Destination(const StormByte::Multimedia::Container& container,
				std::filesystem::path path) noexcept;

			/**
			 * @}
			 */

			/**
			 * @name Run
			 * @{
			 */

			/**
			 * @brief Builds the @ref Plan, starts the coordinator and returns.
			 *
			 * Calls @ref OnConfigure, @ref EmptyPlan, fills tracks from
			 * the fluent map, @ref OnPlan, @ref OnStart, then
			 * @c std::move(*plan) >> demux. Does not block until Done.
			 */
			void Run() noexcept;

			/**
			 * @brief Requests abort. Coordinator ends in @ref Status::Aborted.
			 */
			void Cancel() noexcept;

			/**
			 * @brief Pauses the coordinator.
			 */
			void Pause() noexcept;

			/**
			 * @brief Resumes after @ref Pause.
			 */
			void Resume() noexcept;

			/**
			 * @brief Current lifecycle value.
			 * @return Status.
			 */
			enum Status Status() const noexcept;

			/**
			 * @brief Whether the job failed.
			 * @return true after @ref Status::Error.
			 */
			bool Failed() const noexcept;

			/**
			 * @brief Failure text.
			 * @return Message, or empty.
			 */
			std::optional<std::string> Error() const noexcept;

			/**
			 * @brief Last published percent.
			 * @return 0..100, or empty before the first tick.
			 */
			std::optional<unsigned> Progress() const noexcept;

			/**
			 * @brief true if not failed.
			 * @return Not @ref Failed.
			 */
			explicit operator bool() const noexcept;

			/**
			 * @}
			 */

		protected:
			/**
			 * @brief Allocates the Plan type for this job.
			 * @param source Origin File (moved).
			 * @param container Destination container.
			 * @param destination Output path.
			 * @return Plan of the desired dynamic type, with no tracks yet.
			 *
			 * Override to return a type derived from @ref Plan. Tracks
			 * are filled from the fluent map after this returns.
			 */
			virtual std::unique_ptr<class Plan> EmptyPlan(File&& source,
				const StormByte::Multimedia::Container& container,
				std::filesystem::path destination) const noexcept;

			/**
			 * @brief Allocates the settled-row type.
			 * @return Empty row of the desired dynamic type.
			 *
			 * Override to return a type derived from @ref TrackSettled.
			 * @ref MarkSettled fills it and calls @ref OnSettled.
			 */
			virtual std::unique_ptr<TrackSettled> EmptySettled() const noexcept;

			/**
			 * @brief Last chance to raise ceilings before the job starts.
			 */
			virtual void OnConfigure() noexcept;

			/**
			 * @brief Gate after the Plan is filled and Destination is set.
			 * @return Running to proceed, Error / Aborted / Stopped to bail.
			 */
			virtual enum Status OnStart() noexcept;

			/**
			 * @brief Intention, just before @c plan >> demux.
			 * @param plan Filled Plan (still owned by this job).
			 */
			virtual void OnPlan(const class Plan& plan) noexcept;

			/**
			 * @brief One encode lane finished Encoder open.
			 * @param track Settled row from @ref EmptySettled.
			 */
			virtual void OnSettled(const TrackSettled& track) noexcept;

			/**
			 * @brief Progress tick.
			 * @param percent 0..100.
			 */
			virtual void OnProgress(unsigned percent) noexcept;

			/**
			 * @brief Successful flush.
			 */
			virtual void OnDone() noexcept;

			/**
			 * @brief Hard error.
			 * @param message Error text.
			 */
			virtual void OnError(const std::string& message) noexcept;

			/**
			 * @brief Cancel completed.
			 */
			virtual void OnAborted() noexcept;

		private:
			friend class Engine::Transcode::Engine;
			friend class Track;

			/**
			 * @brief Marks a hard error and cancels the coordinator.
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
			 * @brief Maps one origin stream.
			 * @param in Origin index.
			 * @param out Mux slot.
			 * @param kind Expected type.
			 * @return Fluent track handle.
			 */
			Track AddTrack(int in, int out, Type kind) noexcept;

			/**
			 * @brief Coordinator thread body. Wires steps and waits.
			 */
			void Worker() noexcept;

			/**
			 * @brief Records encoder open into a @ref TrackSettled and fires OnSettled.
			 * @param in Origin stream index of the lane.
			 * @param encoder Encoder that just opened.
			 */
			void MarkSettled(int in, Encoder& encoder) noexcept;

			/**
			 * @brief Publishes percent to Progress() and OnProgress.
			 * @param percent 0..100.
			 */
			void SetProgress(unsigned percent) noexcept;

			/**
			 * @brief Whether @p slot is a mapped track.
			 * @param slot Engine map index.
			 * @return true if the slot exists.
			 */
			bool ValidSlot(std::size_t slot) const noexcept;

			/**
			 * @brief Appends a track filter to @p slot.
			 * @param slot Engine map index.
			 * @param filter Filter instance.
			 */
			void AttachFilter(std::size_t slot, std::shared_ptr<Filter::FFmpeg> filter) noexcept;

			/**
			 * @brief Appends an analytics filter to every encode lane.
			 * @param filter Filter instance.
			 */
			void AttachAnalytics(std::shared_ptr<Filter::FFmpeg> filter) noexcept;

			std::shared_ptr<StormByte::Logger::Log> m_logger;			///< Required logger
			std::unique_ptr<File> m_file;								///< Source until handed to the Plan
			std::shared_ptr<class Plan> m_plan;							///< Intention; shared with the job after Run
			std::unique_ptr<Engine::Transcode::Engine> m_engine;		///< Map and coordinator thread
	};
}
