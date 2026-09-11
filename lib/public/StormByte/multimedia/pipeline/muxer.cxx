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

#include <StormByte/multimedia/pipeline/muxer.hxx>

#include <StormByte/multimedia/buffer/sink.hxx>
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/name_thread.hxx>
#include <StormByte/multimedia/pipeline/demuxer.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/engine/mux/details/container.hxx>
#include <StormByte/multimedia/pipeline/engine/mux/engine.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/remuxer.hxx>
#include <StormByte/multimedia/type.hxx>

#include <chrono>
#include <utility>

using namespace StormByte::Multimedia;
using namespace StormByte::Multimedia::Pipeline;

Muxer::Muxer(const Container& container) noexcept
: Step(Kinds{Kind::Packet}, Kinds{}),
m_container(&container), m_engine(std::make_unique<Engine::Mux::Details::Container>()),
m_closed(false), m_positionNs(-1) {
	if (!container.HasAccess(Access{Operation::Write})) {
		Fail("container does not allow write");
		return;
	}
	Launch();
}

Muxer::~Muxer() noexcept {
	if (m_engine)
		m_engine->Close();
}

Muxer::operator bool() const noexcept {
	return !Failed() && !m_closed.load(std::memory_order_acquire) && m_engine && m_engine->IsOpen();
}

bool Muxer::Closed() const noexcept {
	return m_closed.load(std::memory_order_acquire) || Failed();
}

std::optional<Property::Duration> Muxer::Position() const noexcept {
	const std::int64_t ns = m_positionNs.load(std::memory_order_acquire);
	if (ns < 0)
		return std::nullopt;
	return Property::Duration{std::chrono::nanoseconds{ns}};
}

const Container& Muxer::Destination() const noexcept {
	return *m_container;
}

void Muxer::Fail(std::string reason) noexcept {
	m_closed.store(true, std::memory_order_release);
	if (m_engine)
		m_engine->Close();
	Step::Fail(std::move(reason));
}

void Muxer::Open() noexcept {}

void Muxer::Work(std::shared_ptr<Item> item) noexcept {
	NameThread("STMM:Muxer");

	if (Failed() || !m_engine)
		return;
	auto packet = std::dynamic_pointer_cast<Packet>(item);
	if (!packet) {
		Fail("muxer expected a packet");
		return;
	}
	while (!m_engine->Push(*this, packet)) {
		if (Failed())
			return;
		Wait();
	}
	if (packet->Type() == Type::Video) {
		if (const auto& pts = packet->Pts(); pts)
			m_positionNs.store(pts->Nanoseconds().count(), std::memory_order_release);
	}
}

void Muxer::Finish() noexcept {
	if (m_engine && !Failed())
		m_engine->Flush(*this);
	m_closed.store(true, std::memory_order_release);
}

Encoder& StormByte::Multimedia::Pipeline::operator>>(Encoder& encoder, Muxer& muxer) noexcept {
	static_cast<Step&>(encoder) >> muxer;
	if (muxer.Failed() || encoder.Failed())
		return encoder;
	if (!muxer.m_engine) {
		muxer.Fail("muxer has no backend");
		return encoder;
	}
	muxer.m_engine->ReserveEncoder(muxer, encoder);
	return encoder;
}

Muxer& StormByte::Multimedia::Pipeline::operator>>(Muxer& muxer, const std::filesystem::path& path) noexcept {
	if (muxer.Failed())
		return muxer;
	if (!muxer.m_engine) {
		muxer.Fail("muxer has no backend");
		return muxer;
	}
	muxer.m_engine->BindPath(muxer, path);
	return muxer;
}

Muxer& StormByte::Multimedia::Pipeline::operator>>(Demuxer& demuxer, Muxer& muxer) noexcept {
	static_cast<Step&>(demuxer) >> muxer;
	if (muxer.Failed())
		return muxer;
	if (!demuxer.Plan()) {
		muxer.Fail("demuxer has no plan");
		return muxer;
	}
	if (!muxer.m_engine) {
		muxer.Fail("muxer has no backend");
		return muxer;
	}
	muxer.m_engine->BindAttachments(muxer, demuxer.OriginFile());
	return muxer;
}

Muxer& StormByte::Multimedia::Pipeline::operator>>(Remuxer& remuxer, Muxer& muxer) noexcept {
	static_cast<Step&>(remuxer) >> muxer;
	if (muxer.Failed() || remuxer.Failed())
		return muxer;
	if (!muxer.m_engine) {
		muxer.Fail("muxer has no backend");
		return muxer;
	}
	if (!muxer.m_engine->ReserveRemux(muxer, remuxer))
		return muxer;
	muxer.m_in->Notify(muxer.Wake());
	remuxer.m_out->Bind(remuxer.In(), *muxer.m_in);
	return muxer;
}
