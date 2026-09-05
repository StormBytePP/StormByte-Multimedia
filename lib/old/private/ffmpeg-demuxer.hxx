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
