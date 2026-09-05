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

#include <StormByte/multimedia/engine/stream.hxx>

using namespace StormByte::Multimedia::Engine;

Stream::Stream(const class Codec& codec, enum Type type) noexcept:
m_codec(codec), m_type(type), m_metadata() {
	// If codec is mjpeg, then stream will be set as Video when it is Attachment
	if (codec.Name() == "mjpeg") {
		m_type = Type::Attachment;
	}
}

Multimedia::Type Stream::Type() const noexcept {
	return m_type;
}

const StormByte::Multimedia::Metadata& Stream::Metadata() const noexcept {
	return m_metadata;
}

void Stream::Metadata(class Metadata&& metadata) noexcept {
	m_metadata = std::move(metadata);
}

std::shared_ptr<const StormByte::Multimedia::Context::Generic> Stream::Context() const noexcept {
	return m_context;
}

void Stream::Context(StormByte::Multimedia::Context::Generic&& context) noexcept {
	m_context = context.Move();
}

const class Codec& Stream::Codec() const noexcept {
	return m_codec;
}