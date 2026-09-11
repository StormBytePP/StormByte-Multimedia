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
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/engine/mux/details/container.hxx>
#include <StormByte/multimedia/pipeline/engine/mux/engine.hxx>
#include <StormByte/multimedia/pipeline/mux.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/remux.hxx>
#include <StormByte/multimedia/type.hxx>

#include <StormByte/multimedia/name_thread.hxx>

#include <chrono>
#include <utility>

using namespace StormByte::Multimedia::Pipeline;

Mux::Mux(const StormByte::Multimedia::Container& container) noexcept
: Step(Kinds{Kind::Packet}, Kinds{}),
m_container(&container), m_engine(std::make_unique<Engine::Mux::Details::Container>()),
m_closed(false), m_positionNs(-1) {
	if (!container.HasAccess(Access{Operation::Write})) {
		Fail("container does not allow write");
		return;
	}
	Launch();
}

Mux::~Mux() noexcept {
	if (m_engine)
		m_engine->Close();
}

Mux::operator bool() const noexcept {
	return !Failed() && !m_closed.load(std::memory_order_acquire) && m_engine && m_engine->IsOpen();
}

bool Mux::Closed() const noexcept {
	return m_closed.load(std::memory_order_acquire) || Failed();
}

std::optional<StormByte::Multimedia::Property::Duration> Mux::Position() const noexcept {
	const std::int64_t ns = m_positionNs.load(std::memory_order_acquire);
	if (ns < 0)
		return std::nullopt;
	return StormByte::Multimedia::Property::Duration{std::chrono::nanoseconds{ns}};
}

const StormByte::Multimedia::Container& Mux::Destination() const noexcept {
	return *m_container;
}

void Mux::Fail(std::string reason) noexcept {
	m_closed.store(true, std::memory_order_release);
	if (m_engine)
		m_engine->Close();
	Step::Fail(std::move(reason));
}

void Mux::Open() noexcept {}

void Mux::Work(std::shared_ptr<Item> item) noexcept {
	NameThread("STMM:Mux");

	if (Failed() || !m_engine)
		return;
	auto packet = std::dynamic_pointer_cast<Packet>(item);
	if (!packet) {
		Fail("mux expected a packet");
		return;
	}
	while (!m_engine->Push(*this, packet)) {
		if (Failed())
			return;
		Wait();
	}
	if (packet->Type() == StormByte::Multimedia::Type::Video) {
		if (const auto& pts = packet->Pts(); pts)
			m_positionNs.store(pts->Nanoseconds().count(), std::memory_order_release);
	}
}

void Mux::Finish() noexcept {
	if (m_engine && !Failed())
		m_engine->Flush(*this);
	m_closed.store(true, std::memory_order_release);
}

Encoder& StormByte::Multimedia::Pipeline::operator>>(Encoder& encoder, Mux& mux) noexcept {
	static_cast<Step&>(encoder) >> mux;
	if (mux.Failed() || encoder.Failed())
		return encoder;
	if (!mux.m_engine) {
		mux.Fail("muxer has no backend");
		return encoder;
	}
	mux.m_engine->ReserveEncoder(mux, encoder);
	return encoder;
}

Mux& StormByte::Multimedia::Pipeline::operator>>(Mux& mux, const std::filesystem::path& path) noexcept {
	if (mux.Failed())
		return mux;
	if (!mux.m_engine) {
		mux.Fail("muxer has no backend");
		return mux;
	}
	mux.m_engine->BindPath(mux, path);
	return mux;
}

Mux& StormByte::Multimedia::Pipeline::operator>>(Demux& demux, Mux& mux) noexcept {
	static_cast<Step&>(demux) >> mux;
	if (mux.Failed())
		return mux;
	if (!demux.Plan()) {
		mux.Fail("demuxer has no plan");
		return mux;
	}
	if (!mux.m_engine) {
		mux.Fail("muxer has no backend");
		return mux;
	}
	mux.m_engine->BindAttachments(mux, demux.OriginFile());
	return mux;
}

Mux& StormByte::Multimedia::Pipeline::operator>>(Remux& remux, Mux& mux) noexcept {
	static_cast<Step&>(remux) >> mux;
	if (mux.Failed() || remux.Failed())
		return mux;
	if (!mux.m_engine) {
		mux.Fail("muxer has no backend");
		return mux;
	}
	if (!mux.m_engine->ReserveRemux(mux, remux))
		return mux;
	mux.m_in->Notify(mux.Wake());
	remux.m_out->Bind(remux.In(), *mux.m_in);
	return mux;
}
