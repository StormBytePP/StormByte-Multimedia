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

#include <StormByte/multimedia/engine/backend/demuxer.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFormatContext.hxx>

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	/**
	 * @class Demuxer
	 * @brief FFmpeg implementation of Backend::Demuxer.
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE Demuxer final: public Backend::Demuxer {
		public:
			/**
			 * Default constructor.
			 */
			Demuxer() noexcept = default;

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
			~Demuxer() noexcept override = default;

			/**
			 * Copy assignment (deleted).
			 */
			Demuxer& operator=(const Demuxer& other) = delete;

			/**
			 * Move assignment (deleted).
			 */
			Demuxer& operator=(Demuxer&& other) noexcept = delete;

			/**
			 * Opens @p file via AVFormatContext and builds Engine streams (incl. HDR probe).
			 * @param file Media path.
			 * @return Metadata + streams, or error.
			 */
			ExpectedDemuxerTuple Open(const std::filesystem::path& file) const noexcept override;
	};
}
