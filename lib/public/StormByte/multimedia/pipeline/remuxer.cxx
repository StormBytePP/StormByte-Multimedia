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
#include <StormByte/multimedia/name_thread.hxx>
#include <StormByte/multimedia/stream.hxx>
#include <StormByte/multimedia/pipeline/demuxer.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/remuxer.hxx>
#include <StormByte/multimedia/property/duration.hxx>

#include <format>
#include <string>

using namespace StormByte::Multimedia::Pipeline;
using StormByte::Logger::Level;

namespace {
	std::string Ns(const std::optional<StormByte::Multimedia::Property::Duration>& value) noexcept {
		if (!value)
			return "-";
		return std::format("{}", value->Nanoseconds().count());
	}
}

Remuxer::Remuxer(std::shared_ptr<StormByte::Logger::Log> log, int in) noexcept
: Step(std::move(log), Producer::Remuxer, Kinds{Kind::Packet}, Kinds{Kind::Packet}),
	m_index(in) {
	Launch();
}

Remuxer::~Remuxer() noexcept {
	Halt();
}

void Remuxer::Open() noexcept {
	if (m_index < 0) {
		Fail("remuxer origin is negative");
		return;
	}
	Log(Level::Notice, std::format("open t={}", m_index));
	Step::Open();
}

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
	if (!packet->Serial()) {
		Fail("packet has no serial");
		return;
	}
	if (Sparse(m_index))
		Log(Level::LowLevel, std::format("fwd t={} {}:{} pts={} dts={}",
			packet->Track(), *packet->Serial(), packet->Part(),
			Ns(packet->Pts()), Ns(packet->Dts())));
	MaybeThrottle(m_index);
	m_out->Push(packet);
}

void Remuxer::Finish() noexcept {}

Remuxer& StormByte::Multimedia::Pipeline::operator>>(Demuxer& demuxer, Remuxer& remuxer) noexcept {
	if (remuxer.Failed())
		return remuxer;
	if (!remuxer.m_plan)
		remuxer.m_plan = demuxer.m_plan;
	if (demuxer.Failed()) {
		remuxer.Fail(demuxer.Error().value_or("demuxer failed"));
		return remuxer;
	}
	remuxer.m_in->Notify(remuxer.Wake());
	demuxer.m_out->Bind(remuxer.In(), *remuxer.m_in);
	if (const std::size_t cap = remuxer.InputCeiling(); cap > 0)
		remuxer.m_in->Capacity(remuxer.In(), cap);
	demuxer.Log(Level::Debug, std::format("bind remuxer t={}", remuxer.In()));
	return remuxer;
}

std::string Remuxer::Label() const noexcept {
	if (m_plan) {
		for (const auto& stream : m_plan->Source().Streams()) {
			if (stream.Index() == m_index)
				return "Remuxer(" + std::string(stream.Codec().Name()) + ")";
		}
	}
	return "Remuxer(t=" + std::to_string(m_index) + ")";
}
