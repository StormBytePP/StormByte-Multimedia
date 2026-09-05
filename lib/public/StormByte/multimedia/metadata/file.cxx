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

#include <StormByte/multimedia/metadata/file.hxx>

using StormByte::Multimedia::Metadata::File;

const std::optional<std::string>& File::Title() const noexcept { return m_title; }
const std::optional<std::string>& File::Artist() const noexcept { return m_artist; }
const std::optional<std::string>& File::Album() const noexcept { return m_album; }
const std::optional<std::string>& File::AlbumArtist() const noexcept { return m_albumArtist; }
const std::optional<std::string>& File::Composer() const noexcept { return m_composer; }
const std::optional<std::string>& File::Genre() const noexcept { return m_genre; }
const std::optional<std::string>& File::Comment() const noexcept { return m_comment; }
const std::optional<std::string>& File::Copyright() const noexcept { return m_copyright; }
const std::optional<std::string>& File::Encoder() const noexcept { return m_encoder; }
const std::optional<std::string>& File::Date() const noexcept { return m_date; }
std::optional<unsigned> File::Track() const noexcept { return m_track; }
std::optional<unsigned> File::Disc() const noexcept { return m_disc; }

void File::Title(std::string title) noexcept { m_title = std::move(title); }
void File::Artist(std::string artist) noexcept { m_artist = std::move(artist); }
void File::Album(std::string album) noexcept { m_album = std::move(album); }
void File::AlbumArtist(std::string albumArtist) noexcept { m_albumArtist = std::move(albumArtist); }
void File::Composer(std::string composer) noexcept { m_composer = std::move(composer); }
void File::Genre(std::string genre) noexcept { m_genre = std::move(genre); }
void File::Comment(std::string comment) noexcept { m_comment = std::move(comment); }
void File::Copyright(std::string copyright) noexcept { m_copyright = std::move(copyright); }
void File::Encoder(std::string encoder) noexcept { m_encoder = std::move(encoder); }
void File::Date(std::string date) noexcept { m_date = std::move(date); }
void File::Track(unsigned track) noexcept { m_track = track; }
void File::Disc(unsigned disc) noexcept { m_disc = disc; }
