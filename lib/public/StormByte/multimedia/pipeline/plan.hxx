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

#include <StormByte/buffer/io/buffered_file_reader.hxx>
#include <StormByte/buffer/io/buffered_file_writer.hxx>
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/track.hxx>
#include <StormByte/multimedia/pipeline/typedefs.hxx>
#include <StormByte/multimedia/visibility.h>
#include <StormByte/type_traits.hxx>

#include <filesystem>
#include <memory>
#include <optional>
#include <utility>

/**
 * @namespace StormByte
 * @brief Root namespace of the StormByte C++ suite.
 */
namespace StormByte {
	/**
	 * @namespace StormByte::Multimedia
	 * @brief Multimedia module of the StormByte suite.
	 */
	namespace Multimedia {
		/**
		 * @namespace StormByte::Multimedia::Pipeline
		 * @brief Demux / decode / filter / encode / mux types.
		 *
		 * @ingroup multimedia_pipeline
		 */
		namespace Pipeline {
			class Demuxer;

			/**
			 * @class Plan
			 * @brief Closed job intention: owned reader, owned writer, tracks.
			 *
			 * Octets in and out are BufferedFile leaves taken by &&
			 * (or built from a path) and stored on the heap as the
			 * dynamic type. No slicing. The constructor probes the
			 * reader once into a consultation File snapshot
			 * (streams, attachments, metadata). That File is not the
			 * octet origin. Duration() is not called on it.
			 *
			 * Destination container is resolved from the writer path
			 * extension. There is no Container argument.
			 *
			 * Mux output order is the order of @ref add. `add` is not
			 * idempotent. A track omitted from @ref Tracks is dropped.
			 *
			 * @ref Check tests formation of this intention only. Success
			 * does not mean the tube will run. @ref operator bool is
			 * @ref Check.
			 *
			 * @ingroup multimedia_pipeline
			 */
			class STORMBYTE_MULTIMEDIA_PUBLIC Plan {
				public:
					/**
					 * @name Lifecycle
					 * @{
					 */

					/**
					 * @brief Builds reader and writer from paths.
					 * @param source Input path.
					 * @param destination Output path.
					 */
					Plan(const std::filesystem::path& source,
						const std::filesystem::path& destination) noexcept;

					/**
					 * @brief Builds a reader from @p source and takes @p writer.
					 * @tparam Writer Leaf type derived from BufferedFileWriter.
					 * @param source Input path.
					 * @param writer Output writer (moved).
					 */
					template<typename Writer>
					requires StormByte::Type::DerivedFrom<Writer, StormByte::Buffer::IO::BufferedFileWriter>
					Plan(const std::filesystem::path& source, Writer&& writer) noexcept
					: Plan(StormByte::Buffer::IO::BufferedFileReader{source},
						std::forward<Writer>(writer)) {}

					/**
					 * @brief Takes @p reader and builds a writer on @p destination.
					 * @tparam Reader Leaf type derived from BufferedFileReader.
					 * @param reader Input reader (moved).
					 * @param destination Output path.
					 */
					template<typename Reader>
					requires StormByte::Type::DerivedFrom<Reader, StormByte::Buffer::IO::BufferedFileReader>
					Plan(Reader&& reader, const std::filesystem::path& destination) noexcept
					: Plan(std::forward<Reader>(reader),
						StormByte::Buffer::IO::BufferedFileWriter{destination}) {}

					/**
					 * @brief Takes both leaves. Heap-allocates the dynamic types.
					 * @tparam Reader Leaf type derived from BufferedFileReader.
					 * @tparam Writer Leaf type derived from BufferedFileWriter.
					 * @param reader Input reader (moved).
					 * @param writer Output writer (moved).
					 */
					template<typename Reader, typename Writer>
					requires StormByte::Type::DerivedFrom<Reader, StormByte::Buffer::IO::BufferedFileReader>
						&& StormByte::Type::DerivedFrom<Writer, StormByte::Buffer::IO::BufferedFileWriter>
					Plan(Reader&& reader, Writer&& writer) noexcept
					: Plan(std::unique_ptr<StormByte::Buffer::IO::BufferedFileReader>(
							std::make_unique<std::remove_cvref_t<Reader>>(std::forward<Reader>(reader))),
						std::unique_ptr<StormByte::Buffer::IO::BufferedFileWriter>(
							std::make_unique<std::remove_cvref_t<Writer>>(std::forward<Writer>(writer)))) {}

					Plan(const Plan&) = delete;
					Plan& operator=(const Plan&) = delete;

					/**
					 * @brief Move constructor.
					 * @param other Plan to take.
					 */
					Plan(Plan&& other) noexcept = default;

					/**
					 * @brief Destructor.
					 */
					virtual ~Plan() noexcept = default;

					/**
					 * @brief Move assignment.
					 * @param other Plan to take.
					 * @return *this.
					 */
					Plan& operator=(Plan&& other) noexcept = default;

					/**
					 * @}
					 */

					/**
					 * @brief Deep copy is not supported.
					 */
					std::unique_ptr<Plan> Clone() const = delete;

					/**
					 * @brief Move into a new pointer.
					 * @return Owning pointer to the moved @ref Plan.
					 */
					inline std::unique_ptr<Plan> Move() {
						return std::make_unique<Plan>(std::move(*this));
					}

					/**
					 * @brief Same as @ref Check having a value.
					 * @return true if this intention is well formed.
					 */
					explicit operator bool() const noexcept;

					/**
					 * @name Job
					 * @{
					 */

					/**
					 * @brief Origin reader owned by this Plan.
					 * @return Reader.
					 */
					StormByte::Buffer::IO::BufferedFileReader& Reader() noexcept;

					/**
					 * @brief Origin reader owned by this Plan.
					 * @return Reader.
					 */
					const StormByte::Buffer::IO::BufferedFileReader& Reader() const noexcept;

					/**
					 * @brief Consultation snapshot taken in the constructor.
					 * @return File snapshot (streams, attachments, metadata).
					 *
					 * Undefined if `!*this`. Not an octet source. Do not call
					 * File::Duration() on it while the tube reads.
					 */
					const StormByte::Multimedia::File& Snapshot() const noexcept;

					/**
					 * @brief Destination container (registry, from writer extension).
					 * @return Container.
					 */
					inline const class Container& Container() const noexcept {
						return *m_container;
					}

					/**
					 * @brief Destination writer owned by this Plan.
					 * @return Writer.
					 */
					StormByte::Buffer::IO::BufferedFileWriter& Writer() noexcept;

					/**
					 * @brief Destination writer owned by this Plan.
					 * @return Writer.
					 */
					const StormByte::Buffer::IO::BufferedFileWriter& Writer() const noexcept;

					/**
					 * @brief Output path forwarded from the writer.
					 * @return Path.
					 */
					inline const std::filesystem::path& Path() const noexcept {
						return m_writer->Path();
					}

					/**
					 * @brief Tube tracks. Index is mux slot.
					 * @return Track list.
					 */
					inline const class Tracks& Tracks() const noexcept {
						return m_tracks;
					}

					/**
					 * @brief Appends a clone of @p track. Not idempotent.
					 * @param track Track to copy.
					 */
					inline void add(const Track& track) {
						m_tracks.add(track);
					}

					/**
					 * @brief Appends @p track. Not idempotent. Order is mux order.
					 * @param track Track to take.
					 */
					inline void add(Track&& track) {
						m_tracks.add(std::move(track));
					}

					/**
					 * @}
					 */

					/**
					 * @brief Whether this intention is well formed.
					 *
					 * Fails when: the Plan was moved-from; the constructor
					 * snapshot failed; @ref Path is empty; @ref Tracks is
					 * empty; @ref Track::In is negative; @ref Track::Type
					 * is @ref Type::Unknown; an attachment MIME is empty
					 * or is not an exact type/subtype, a type-star
					 * category, or star-star (all).
					 * Duplicate @ref Track::In is allowed. Encode knobs on a
					 * remux track (`Codec() == nullptr`) are ignored.
					 *
					 * @return Empty value, or @ref PlanException.
					 */
					virtual CheckResult Check() const;

				private:
					/**
					 * @brief Takes already heap-allocated leaves. No slicing.
					 * @param reader Owned origin.
					 * @param writer Owned sink.
					 */
					Plan(std::unique_ptr<StormByte::Buffer::IO::BufferedFileReader> reader,
						std::unique_ptr<StormByte::Buffer::IO::BufferedFileWriter> writer) noexcept;

					/**
					 * @brief Resolves the registry container from the writer path.
					 * @param writer Sink whose Path() has the extension.
					 * @return Registry container pointer, or nullptr.
					 */
					static const class Container* ContainerFromWriter(
						const StormByte::Buffer::IO::BufferedFileWriter& writer) noexcept;

					std::unique_ptr<StormByte::Buffer::IO::BufferedFileReader> m_reader;	///< Owned origin octets
					std::unique_ptr<StormByte::Buffer::IO::BufferedFileWriter> m_writer;	///< Owned sink octets
					std::optional<StormByte::Multimedia::File> m_snapshot;				///< Constructor probe
					const class Container* m_container;										///< Registry destination
					class Tracks m_tracks;													///< Tube tracks; index is mux slot
			};

			/**
			 * @brief Hands @p plan to @p demuxer. Not a Step bind.
			 * @param plan Intention (moved).
			 * @param demuxer Destination.
			 * @return @p demuxer.
			 *
			 * Stores the Plan on the Demuxer and wakes it. A second Plan
			 * is Fail. Does not call @ref Plan::Check and does not bind hoppers.
			 */
			STORMBYTE_MULTIMEDIA_PUBLIC Demuxer& operator>>(Plan&& plan, Demuxer& demuxer) noexcept;
		}
	}
}
