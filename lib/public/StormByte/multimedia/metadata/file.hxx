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

#include <optional>
#include <string>

namespace StormByte::Multimedia {
	/**
	 * @class File
	 * @brief Public media file snapshot.
	 */
	class File;
}

namespace StormByte::Multimedia::Detail {
	/**
	 * @class Probe
	 * @brief Private metadata probe.
	 */
	class Probe;
}

/**
 * @namespace StormByte::Multimedia::Metadata
 * @brief Snapshot metadata for files and streams.
 */
namespace StormByte::Multimedia::Metadata {
	/**
	 * @class File
	 * @brief Container-level tags and header fields captured at Open.
	 *
	 * Duration that requires a demux scan lives on Multimedia::File, not here.
	 */
	class STORMBYTE_MULTIMEDIA_PUBLIC File {
		public:
			/**
			 * @brief Copy constructor.
			 */
			File(const File&) = default;

			/**
			 * @brief Move constructor.
			 */
			File(File&&) noexcept = default;

			/**
			 * @brief Destructor.
			 */
			~File() = default;

			/**
			 * @brief Copy assignment.
			 * @return *this.
			 */
			File& operator=(const File&) = default;

			/**
			 * @brief Move assignment.
			 * @return *this.
			 */
			File& operator=(File&&) noexcept = default;

			/**
			 * @brief Work title.
			 * @return Title, or empty.
			 */
			const std::optional<std::string>& Title() const noexcept;

			/**
			 * @brief Main artist.
			 * @return Artist, or empty.
			 */
			const std::optional<std::string>& Artist() const noexcept;

			/**
			 * @brief Album / set name.
			 * @return Album, or empty.
			 */
			const std::optional<std::string>& Album() const noexcept;

			/**
			 * @brief Album artist if different from Artist.
			 * @return Album artist, or empty.
			 */
			const std::optional<std::string>& AlbumArtist() const noexcept;

			/**
			 * @brief Composer.
			 * @return Composer, or empty.
			 */
			const std::optional<std::string>& Composer() const noexcept;

			/**
			 * @brief Genre.
			 * @return Genre, or empty.
			 */
			const std::optional<std::string>& Genre() const noexcept;

			/**
			 * @brief Comment.
			 * @return Comment, or empty.
			 */
			const std::optional<std::string>& Comment() const noexcept;

			/**
			 * @brief Copyright notice.
			 * @return Copyright, or empty.
			 */
			const std::optional<std::string>& Copyright() const noexcept;

			/**
			 * @brief Encoder identification.
			 * @return Encoder, or empty.
			 */
			const std::optional<std::string>& Encoder() const noexcept;

			/**
			 * @brief Date / year tag.
			 * @return Date, or empty.
			 */
			const std::optional<std::string>& Date() const noexcept;

			/**
			 * @brief Track number.
			 * @return Track, or empty.
			 */
			std::optional<unsigned> Track() const noexcept;

			/**
			 * @brief Disc number.
			 * @return Disc, or empty.
			 */
			std::optional<unsigned> Disc() const noexcept;

		private:
			friend class StormByte::Multimedia::File;
			friend class StormByte::Multimedia::Detail::Probe;

			std::optional<std::string> m_title;			///< title
			std::optional<std::string> m_artist;		///< artist
			std::optional<std::string> m_album;			///< album
			std::optional<std::string> m_albumArtist;	///< album_artist
			std::optional<std::string> m_composer;		///< composer
			std::optional<std::string> m_genre;			///< genre
			std::optional<std::string> m_comment;		///< comment
			std::optional<std::string> m_copyright;		///< copyright
			std::optional<std::string> m_encoder;		///< encoder
			std::optional<std::string> m_date;			///< date
			std::optional<unsigned> m_track;			///< track
			std::optional<unsigned> m_disc;				///< disc

			/**
			 * @brief Empty metadata.
			 */
			File() noexcept = default;

			/**
			 * @brief Sets the title.
			 * @param title Title tag.
			 */
			void Title(std::string title) noexcept;

			/**
			 * @brief Sets the artist.
			 * @param artist Artist tag.
			 */
			void Artist(std::string artist) noexcept;

			/**
			 * @brief Sets the album.
			 * @param album Album tag.
			 */
			void Album(std::string album) noexcept;

			/**
			 * @brief Sets the album artist.
			 * @param albumArtist Album artist tag.
			 */
			void AlbumArtist(std::string albumArtist) noexcept;

			/**
			 * @brief Sets the composer.
			 * @param composer Composer tag.
			 */
			void Composer(std::string composer) noexcept;

			/**
			 * @brief Sets the genre.
			 * @param genre Genre tag.
			 */
			void Genre(std::string genre) noexcept;

			/**
			 * @brief Sets the comment.
			 * @param comment Comment tag.
			 */
			void Comment(std::string comment) noexcept;

			/**
			 * @brief Sets the copyright.
			 * @param copyright Copyright tag.
			 */
			void Copyright(std::string copyright) noexcept;

			/**
			 * @brief Sets the encoder.
			 * @param encoder Encoder tag.
			 */
			void Encoder(std::string encoder) noexcept;

			/**
			 * @brief Sets the date.
			 * @param date Date tag.
			 */
			void Date(std::string date) noexcept;

			/**
			 * @brief Sets the track number.
			 * @param track Track number.
			 */
			void Track(unsigned track) noexcept;

			/**
			 * @brief Sets the disc number.
			 * @param disc Disc number.
			 */
			void Disc(unsigned disc) noexcept;
	};
}
