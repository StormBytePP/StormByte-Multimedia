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

#include <StormByte/multimedia/visibility.h>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace StormByte::Multimedia::Engine::Backend::FFmpeg
 * @brief Private RAII wrappers over libav*.
 */
namespace StormByte::Multimedia::Engine::Backend::FFmpeg {
	class AVCodecParameters;

	/**
	 * @class AVStream
	 * @brief Non-owning view of an ::AVStream (owned by AVFormatContext).
	 */
	class STORMBYTE_MULTIMEDIA_PRIVATE AVStream {
		public:
			/**
			 * @brief Binds a raw stream pointer (not owned).
			 * @param stream Raw stream pointer.
			 */
			explicit AVStream(::AVStream* stream) noexcept;

			/**
			 * @brief Copy constructor (deleted).
			 */
			AVStream(const AVStream&) = delete;

			/**
			 * @brief Move constructor.
			 * @param other Source view.
			 */
			AVStream(AVStream&& other) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~AVStream() noexcept = default;

			/**
			 * @brief Copy assignment (deleted).
			 * @return *this.
			 */
			AVStream& operator=(const AVStream&) = delete;

			/**
			 * @brief Move assignment.
			 * @param other Source view.
			 * @return *this.
			 */
			AVStream& operator=(AVStream&& other) noexcept = default;

			/**
			 * @brief Orders by stream index (for std::set).
			 * @param other Other stream.
			 * @return true if this index is less.
			 */
			bool operator<(const AVStream& other) const noexcept;

			/**
			 * @brief Stream index.
			 * @return Index, or -1.
			 */
			int Index() const noexcept;

			/**
			 * @brief Stream media type.
			 * @return AVMediaType as int.
			 */
			int Type() const noexcept;

			/**
			 * @brief Copy of codec parameters.
			 * @return Parameters wrapper.
			 */
			AVCodecParameters CodecParameters() const noexcept;

			/**
			 * @brief Stream time base.
			 * @return Time base.
			 */
			AVRational TimeBase() const noexcept;

			/**
			 * @brief Estimated FPS (avg or r_frame_rate).
			 * @return FPS, or 0.
			 */
			double FrameRate() const noexcept;

			/**
			 * @brief Looks up a stream metadata tag.
			 * @param key Dictionary key (e.g. `"language"`).
			 * @return Value, or nullptr if missing.
			 */
			const char* Tag(const char* key) const noexcept;

			/**
			 * @brief Raw FFmpeg disposition bits.
			 * @return `AV_DISPOSITION_*` mask, or 0.
			 */
			int Disposition() const noexcept;

		private:
			::AVStream* m_stream = nullptr;	///< Non-owning
	};
}
