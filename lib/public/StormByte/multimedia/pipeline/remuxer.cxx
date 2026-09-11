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

#include <StormByte/multimedia/pipeline/remuxer.hxx>

#include <StormByte/multimedia/buffer/sink.hxx>
#include <StormByte/multimedia/name_thread.hxx>
#include <StormByte/multimedia/pipeline/demuxer.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>

using namespace StormByte::Multimedia::Pipeline;

Remuxer::Remuxer(int in) noexcept
: Step(Kinds{Kind::Packet}, Kinds{Kind::Packet}), m_index(in), m_demuxer(nullptr) {
	Launch();
}

Remuxer::~Remuxer() noexcept = default;

void Remuxer::Fail(std::string reason) noexcept {
	Step::Fail(std::move(reason));
}

void Remuxer::Open() noexcept {}

void Remuxer::Work(std::shared_ptr<Item> item) noexcept {
	NameThread("STMM:Remuxer:" + std::to_string(m_index));
	if (Failed())
		return;
	auto packet = std::dynamic_pointer_cast<Packet>(item);
	if (!packet) {
		Fail("remuxer expected a packet");
		return;
	}
	if (packet->Track() != m_index)
		return;
	m_out->Push(packet);
}

void Remuxer::Finish() noexcept {}

Remuxer& StormByte::Multimedia::Pipeline::operator>>(Demuxer& demuxer, Remuxer& remuxer) noexcept {
	if (remuxer.Failed())
		return remuxer;
	static_cast<Step&>(demuxer) >> remuxer;
	if (demuxer.Failed()) {
		remuxer.Fail(demuxer.Error().value_or("demuxer is not open"));
		return remuxer;
	}
	if (remuxer.m_index < 0) {
		remuxer.Fail("remuxer origin is negative");
		return remuxer;
	}
	remuxer.m_demuxer = &demuxer;
	remuxer.m_in->Notify(remuxer.Wake());
	demuxer.m_out->Bind(remuxer.In(), *remuxer.m_in);
	return remuxer;
}
