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

#include <StormByte/multimedia/buffer/sink.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/mux.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/remux.hxx>
#include <StormByte/multimedia/name_thread.hxx>

using namespace StormByte::Multimedia::Pipeline;

Remux::Remux(int in) noexcept
: Step(Kinds{Kind::Packet}, Kinds{Kind::Packet}), m_index(in), m_demux(nullptr) {
	Launch();
}

Remux::~Remux() noexcept = default;

void Remux::Fail(std::string reason) noexcept {
	Step::Fail(std::move(reason));
}

void Remux::Open() noexcept {}

void Remux::Work(std::shared_ptr<Item> item) noexcept {
	NameThread("STMM:Remux:" + std::to_string(m_index));
	if (Failed())
		return;
	auto packet = std::dynamic_pointer_cast<Packet>(item);
	if (!packet) {
		Fail("remux expected a packet");
		return;
	}
	if (packet->Track() != m_index)
		return;
	m_out->Push(packet);
}

void Remux::Finish() noexcept {}

Remux& StormByte::Multimedia::Pipeline::operator>>(Demux& demux, Remux& remux) noexcept {
	if (remux.Failed())
		return remux;
	static_cast<Step&>(demux) >> remux;
	if (demux.Failed()) {
		remux.Fail(demux.Error().value_or("demuxer is not open"));
		return remux;
	}
	if (remux.m_index < 0) {
		remux.Fail("remux origin is negative");
		return remux;
	}
	remux.m_demux = &demux;
	remux.m_in->Notify(remux.Wake());
	demux.m_out->Bind(remux.In(), *remux.m_in);
	return remux;
}
