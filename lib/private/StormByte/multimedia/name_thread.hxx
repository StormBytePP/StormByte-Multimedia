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

#include <StormByte/string.hxx>

#include <string>

#ifdef LINUX
	#include <pthread.h>
#elif defined(MACOS)
	#include <pthread.h>
#elif defined(WINDOWS)
	#include <windows.h>
#endif

/**
 * @brief Sets the current thread name for debuggers and process tools.
 * @param name Host name. Linux keeps the first 15 bytes.
 */
inline void NameThread(const std::string& name) noexcept {
	if (name.empty())
		return;
#ifdef LINUX
	char buf[16]{};
	const std::size_t n = name.size() < 15 ? name.size() : 15;
	name.copy(buf, n);
	buf[n] = '\0';
	::pthread_setname_np(::pthread_self(), buf);
#elif defined(MACOS)
	::pthread_setname_np(name.c_str());
#elif defined(WINDOWS)
	std::wstring wide;
	try {
		wide = StormByte::String::UTF8Decode(name);
	}
	catch (...) {
		return;
	}
	if (wide.empty())
		return;
	::SetThreadDescription(::GetCurrentThread(), wide.c_str());
#endif
}
