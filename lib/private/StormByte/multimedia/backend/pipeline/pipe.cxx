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

#include <StormByte/multimedia/backend/pipeline/pipe.hxx>

#include <utility>

using StormByte::Multimedia::Backend::Pipeline::Pipe;

Pipe::Pipe(std::condition_variable& wake) noexcept
:	m_wake(&wake) {}

Pipe::ItemSink& Pipe::In() noexcept {
	return m_in;
}

const Pipe::ItemSink& Pipe::In() const noexcept {
	return m_in;
}

Pipe::ItemSink& Pipe::Out() noexcept {
	return m_out;
}

const Pipe::ItemSink& Pipe::Out() const noexcept {
	return m_out;
}

void Pipe::Capacity(int track, std::size_t n) noexcept {
	m_in.Capacity(track, n);
}

void Pipe::Listen() noexcept {
	m_in.Notify(*m_wake);
}

void Pipe::Close() noexcept {
	m_in.Eof();
	m_out.Eof();
}

bool Pipe::Ready() const noexcept {
	return m_in.Ready();
}

bool Pipe::InputEof() const noexcept {
	return m_in.EoF();
}

Pipe::Lane::Lane(Pipe& from, int track) noexcept
:	m_from(&from), m_track(track) {}

Pipe::Lane Pipe::To(int track) noexcept {
	return Lane(*this, track);
}

Pipe& Pipe::Lane::operator>>(Pipe& dest) noexcept {
	dest.Listen();
	m_from->m_out.Bind(m_track, dest.m_in);
	return dest;
}

Pipe& Pipe::operator>>(Pipe& dest) noexcept {
	dest.Listen();
	m_out.Bind(dest.m_in);
	return dest;
}

Pipe& Pipe::operator>>(Item::PointerType& item) noexcept {
	item = m_in.Pop();
	return *this;
}

Pipe& Pipe::operator<<(Item::PointerType item) noexcept {
	if (!item)
		return *this;
	const int key = item->Track();
	m_out.Push(key, std::move(item));
	return *this;
}

Pipe& StormByte::Multimedia::Backend::Pipeline::operator>>(Pipe::Item::PointerType& item, Pipe& pipe) noexcept {
	pipe << std::move(item);
	return pipe;
}

Pipe& StormByte::Multimedia::Backend::Pipeline::operator>>(Pipe::Item::PointerType&& item, Pipe& pipe) noexcept {
	pipe << std::move(item);
	return pipe;
}
