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

#include <StormByte/multimedia/pipeline/filters/chain.hxx>

#include <bit>
#include <cassert>

using StormByte::Multimedia::Pipeline::Filter::Chain;
using StormByte::Multimedia::Pipeline::Filter::FFmpeg;
using StormByte::Multimedia::Pipeline::Filter::Origin;
using StormByte::Multimedia::Pipeline::Filter::OriginCount;

std::size_t Chain::Index(Origin origin) noexcept {
	const auto bit = static_cast<unsigned>(origin);
	if (bit == 0 || (bit & (bit - 1)) != 0)
		return OriginCount;
	const std::size_t index = static_cast<std::size_t>(std::countr_zero(bit));
	return index < OriginCount ? index : OriginCount;
}

void Chain::Add(std::unique_ptr<FFmpeg> node) noexcept {
	if (m_armed || m_failed || !node)
		return;
	m_nodes.push_back(std::move(node));
}

std::size_t Chain::Size() const noexcept {
	return m_nodes.size();
}

void Chain::Fail(std::string reason) noexcept {
	m_failed = true;
	m_reason = std::move(reason);
	m_armed = false;
}

bool Chain::Failed() const noexcept {
	return m_failed;
}

std::string Chain::ErrorStr() const noexcept {
	assert(Failed());
	if (!Failed())
		return {};
	return m_reason;
}

void Chain::EnsureArmed() noexcept {
	if (Failed() || m_armed)
		return;
	Reset();
}

void Chain::MaybeDisarm() noexcept {
	if (Failed()) {
		m_armed = false;
		return;
	}
	if (m_eof.HasNone(m_eof))
		return;
	if (m_eof.Has(m_seen))
		m_armed = false;
}

void Chain::Reset() noexcept {
	m_failed = false;
	m_reason.clear();
	m_eof = {};
	m_eofFlushed = {};
	m_seen = {};
	m_calls = {};
	m_armed = true;
	for (auto& node : m_nodes) {
		node->Reset();
		if (node->Failed()) {
			Fail(node->ErrorStr());
			return;
		}
	}
}

void Chain::Call(Pipeline::Frame& frame, Origin origin) noexcept {
	if (Failed())
		return;
	EnsureArmed();
	if (Failed())
		return;
	const std::size_t i = Index(origin);
	if (i >= OriginCount)
		return;
	m_seen |= Flags(origin);
	for (auto& node : m_nodes) {
		node->Call(frame, origin);
		if (node->Failed()) {
			Fail(node->ErrorStr());
			return;
		}
	}
	++m_calls[i];
	if (m_calls[i] >= FlushInterval)
		Flush(origin);
}

void Chain::Call(class Pipeline::Packet& packet, Origin origin) noexcept {
	if (Failed())
		return;
	EnsureArmed();
	if (Failed())
		return;
	const std::size_t i = Index(origin);
	if (i >= OriginCount)
		return;
	m_seen |= Flags(origin);
	for (auto& node : m_nodes) {
		node->Call(packet, origin);
		if (node->Failed()) {
			Fail(node->ErrorStr());
			return;
		}
	}
	++m_calls[i];
	if (m_calls[i] >= FlushInterval)
		Flush(origin);
}

void Chain::Eof(Origin origin) noexcept {
	if (Failed())
		return;
	EnsureArmed();
	if (Failed())
		return;
	if (m_eof.Has(origin))
		return;
	for (auto& node : m_nodes) {
		node->Eof(origin);
		if (node->Failed()) {
			Fail(node->ErrorStr());
			return;
		}
	}
	m_eof |= Flags(origin);
	Flush(origin);
	MaybeDisarm();
}

void Chain::Flush(Origin origin) noexcept {
	if (Failed())
		return;
	const std::size_t i = Index(origin);
	if (i >= OriginCount)
		return;
	if (m_eofFlushed.Has(origin))
		return;
	if (!m_eof.Has(origin) && m_calls[i] < FlushInterval)
		return;
	for (auto& node : m_nodes) {
		node->Flush(origin);
		if (node->Failed()) {
			Fail(node->ErrorStr());
			return;
		}
	}
	m_calls[i] = 0;
	if (m_eof.Has(origin))
		m_eofFlushed |= Flags(origin);
}
