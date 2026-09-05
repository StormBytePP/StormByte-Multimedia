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

#include <StormByte/multimedia/context/property/color.hxx>

using namespace StormByte::Multimedia::Context::Property;

Color::Color(const std::string& pix_fmt, const std::string& range, const std::string& space,
			const std::string& primaries, const std::string& transfer) noexcept:
m_pix_fmt(pix_fmt), m_range(range), m_space(space), m_primaries(primaries), m_transfer(transfer) {}

Color::Color(std::string&& pix_fmt, std::string&& range, std::string&& space) noexcept:
m_pix_fmt(std::move(pix_fmt)), m_range(std::move(range)), m_space(std::move(space)) {}

const std::string& Color::PixelFormat() const noexcept {
	return m_pix_fmt;
}

const std::string& Color::Range() const noexcept {
	return m_range;
}

const std::string& Color::Space() const noexcept {
	return m_space;
}

const std::string& Color::Transfer() const noexcept {
	return m_transfer;
}

const std::string& Color::Primaries() const noexcept {
	return m_primaries;
}

bool Color::IsHDR10Possible() const noexcept {
	return (m_pix_fmt == "yuv420p10le" || m_pix_fmt == "yuv422p10le" || m_pix_fmt == "yuv444p10le") &&
		   (m_range == "tv" || m_range == "full") &&
		   (m_space == "bt2020nc" || m_space == "bt2020c") &&
		   (m_primaries == "bt2020") &&
		   (m_transfer == "smpte2084");
}

bool Color::IsHLGPossible() const noexcept {
	return (m_pix_fmt == "yuv420p10le" || m_pix_fmt == "yuv422p10le" || m_pix_fmt == "yuv444p10le") &&
		   (m_range == "tv" || m_range == "full") &&
		   (m_space == "bt2020nc" || m_space == "bt2020c") &&
		   (m_primaries == "bt2020") &&
		   (m_transfer == "arib-std-b67");
}