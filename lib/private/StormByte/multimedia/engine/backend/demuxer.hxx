/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/multimedia/engine/backend/typedefs.hxx>
#include <StormByte/multimedia/engine/streams.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>
#include <tuple>

/**
 * @namespace Backend
 * @brief Internal demuxer backends.
 */
namespace StormByte::Multimedia::Engine::Backend {
	/**
	 * @class Demuxer
	 * @brief Abstract backend demuxer (Open → metadata + streams).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Demuxer {
		public:
			/**
			 * Copy constructor (deleted).
			 */
			Demuxer(const Demuxer& other) = delete;

			/**
			 * Move constructor (deleted).
			 */
			Demuxer(Demuxer&& other) noexcept = delete;

			/**
			 * Destructor.
			 */
			virtual ~Demuxer() noexcept = default;

			/**
			 * Copy assignment (deleted).
			 */
			Demuxer& operator=(const Demuxer& other) = delete;

			/**
			 * Move assignment (deleted).
			 */
			Demuxer& operator=(Demuxer&& other) noexcept = delete;

			/**
			 * Opens @p file and extracts container metadata and streams.
			 * @param file Path to the media file.
			 * @return Metadata + streams, or DemuxerException.
			 */
			virtual ExpectedDemuxerTuple Open(const std::filesystem::path& file) const noexcept = 0;

		protected:
			/**
			 * Protected default constructor for derived backends.
			 */
			Demuxer() noexcept = default;
	};
}
