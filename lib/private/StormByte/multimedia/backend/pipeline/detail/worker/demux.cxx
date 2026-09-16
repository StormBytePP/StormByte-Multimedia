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

#include <StormByte/multimedia/backend/pipeline/detail/worker/demux.hxx>
#include <StormByte/multimedia/backend/pipeline/demuxer.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/name_thread.hxx>
#include <StormByte/multimedia/pipeline/demuxer.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/plan.hxx>

#include <format>
#include <memory>

namespace StormByte::Multimedia::Backend::Pipeline::Detail::Worker {
	using StormByte::Multimedia::Pipeline::CheckResult;
	using StormByte::Multimedia::Pipeline::Demuxer;
	using StormByte::Multimedia::Pipeline::Item;
	using StormByte::Multimedia::Pipeline::Packet;
	using StormByte::Logger::Level;

	Demux::Demux(Demuxer& owner) noexcept
	:	StormByte::Multimedia::Backend::Pipeline::Worker(owner.Face()),
		m_owner(owner) {}

	void Demux::Setup() noexcept {
		NameThread("STMM:Demuxer");
		m_owner.WaitForPlan();
		if (Stopping() || !m_owner.Plan())
			return;

		if (const CheckResult check = m_owner.Plan()->Check(); !check) {
			Fail((*check.error()).what());
			return;
		}

		m_owner.m_backend = std::make_unique<StormByte::Multimedia::Backend::Pipeline::Demuxer>();
		if (!m_owner.m_backend->Open(m_owner))
			return;

		m_owner.m_eof = false;
		m_owner.m_positionNs.store(-1, std::memory_order_release);
		m_owner.m_nextSerial.clear();
		Log(Level::Notice, std::format("open {}", m_owner.OriginFile().Path().string()));
	}

	void Demux::Process(Item::PointerType) noexcept {
		if (!m_owner.m_backend || !m_owner.m_backend->IsOpen()) {
			Ended();
			return;
		}

		if (Stopping())
			return;

		Packet::PointerType packet = m_owner.m_backend->Read(m_owner);
		if (m_owner.Failed())
			return;
		if (!packet) {
			m_owner.ReachedEof();
			m_owner.m_lookOut.Eof();
			Ended();
			return;
		}

		if (const auto& pts = packet->Pts(); pts)
			m_owner.m_positionNs.store(pts->Nanoseconds().count(), std::memory_order_release);
		if (auto copy = m_owner.CloneItem(*packet))
			m_owner.m_lookOut.Push(packet->Track(), std::move(copy));
		Emit(std::move(packet));
	}

	void Demux::Flush() noexcept {}
}
