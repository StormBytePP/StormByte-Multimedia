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

#include <StormByte/multimedia/metadata.hxx>

extern "C" {
	#include <libavformat/avformat.h>
}

/**
 * @namespace FFmpeg
 * @brief Internal FFmpeg wrappers.
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
			 * @param stream Raw stream pointer (not owned).
			 */
			explicit AVStream(::AVStream* stream) noexcept;

			/**
			 * Copy constructor (deleted).
			 */
			AVStream(const AVStream&) = delete;

			/**
			 * Move constructor.
			 */
			AVStream(AVStream&& other) noexcept = default;

			/**
			 * Destructor.
			 */
			~AVStream() noexcept = default;

			/**
			 * Copy assignment (deleted).
			 */
			AVStream& operator=(const AVStream&) = delete;

			/**
			 * Move assignment.
			 */
			AVStream& operator=(AVStream&& other) noexcept = default;

			/**
			 * Orders by stream index (for std::set).
			 * @param other Other stream.
			 * @return true if this index is less.
			 */
			bool operator<(const AVStream& other) const noexcept;

			/**
			 * @return Stream index, or -1.
			 */
			int Index() const noexcept;

			/**
			 * @return AVMediaType as int.
			 */
			int Type() const noexcept;

			/**
			 * @return Copy of codec parameters.
			 */
			AVCodecParameters CodecParameters() const noexcept;

			/**
			 * @return Stream time base.
			 */
			AVRational TimeBase() const noexcept;

			/**
			 * @return Estimated FPS (avg or r_frame_rate), or 0.
			 */
			double FrameRate() const noexcept;

			/**
			 * @return Stream-level metadata tags.
			 */
			StormByte::Multimedia::Metadata Metadata() const noexcept;

		private:
			::AVStream* m_stream = nullptr;	///< Non-owning
	};
}
