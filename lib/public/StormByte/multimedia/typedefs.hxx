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

#include <StormByte/expected.hxx>
#include <StormByte/multimedia/exception.hxx>

#include <functional>
#include <vector>

/**
 * @namespace StormByte::Multimedia
 * @brief Public media types: codecs, containers, registry and stream kinds.
 */
namespace StormByte::Multimedia {
	class Codec;
	class Container;
	class File;
	class Stream;

	using ExpectedCodec = StormByte::Expected<const Codec&, CodecNotFoundException>;				///< Result of FindCodec
	using ExpectedContainer = StormByte::Expected<const Container&, ContainerNotFoundException>;	///< Result of FindContainer
	using ExpectedFile = StormByte::Expected<File, FileOpenErrorException>;							///< Result of OpenFile
	using CodecRefs = std::vector<std::reference_wrapper<const Codec>>;								///< List of codec references
	using ContainerRefs = std::vector<std::reference_wrapper<const Container>>;						///< List of container references
	using Streams = std::vector<Stream>;															///< Ordered streams
}
