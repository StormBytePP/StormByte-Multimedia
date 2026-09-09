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

#include <StormByte/multimedia/pipeline/engine/packet/engine.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>

#include <utility>

using namespace StormByte::Multimedia::Pipeline;
using StormByte::Multimedia::Type;

Packet::Packet() noexcept
: Item(-1, Type::Unknown, Kind::Packet, Producer::Demux), m_keyFrame(false) {}

Packet::Packet(int track, enum Type type, enum Producer producer, StormByte::Buffer::FIFO payload,
	std::optional<StormByte::Multimedia::Property::Duration> pts,
	std::optional<StormByte::Multimedia::Property::Duration> dts,
	std::optional<StormByte::Multimedia::Property::Duration> duration,
	bool key_frame,
	std::vector<SideData> attachments) noexcept
: Item(track, type, Kind::Packet, producer),
	m_payload(std::move(payload)),
	m_pts(std::move(pts)), m_dts(std::move(dts)), m_duration(std::move(duration)),
	m_keyFrame(key_frame), m_attachments(std::move(attachments)) {}

Packet::Packet(const Packet& other) noexcept
: Item(other),
	m_payload(other.m_payload),
	m_pts(other.m_pts),
	m_dts(other.m_dts),
	m_duration(other.m_duration),
	m_keyFrame(other.m_keyFrame),
	m_attachments(other.m_attachments) {
	if (other.m_engine)
		m_engine = std::make_unique<Engine::Packet::Engine>(*other.m_engine);
}

Packet::Packet(Packet&& other) noexcept
: Item(std::move(other)),
	m_payload(std::move(other.m_payload)),
	m_pts(std::move(other.m_pts)),
	m_dts(std::move(other.m_dts)),
	m_duration(std::move(other.m_duration)),
	m_keyFrame(other.m_keyFrame),
	m_attachments(std::move(other.m_attachments)),
	m_engine(std::move(other.m_engine)) {
	other.BecomeEmpty();
}

Packet::~Packet() noexcept = default;

Packet& Packet::operator=(const Packet& other) noexcept {
	if (this == &other)
		return *this;
	Item::operator=(other);
	m_payload = other.m_payload;
	m_pts = other.m_pts;
	m_dts = other.m_dts;
	m_duration = other.m_duration;
	m_keyFrame = other.m_keyFrame;
	m_attachments = other.m_attachments;
	if (other.m_engine)
		m_engine = std::make_unique<Engine::Packet::Engine>(*other.m_engine);
	else
		m_engine.reset();
	return *this;
}

Packet& Packet::operator=(Packet&& other) noexcept {
	if (this == &other)
		return *this;
	Item::operator=(std::move(other));
	m_payload = std::move(other.m_payload);
	m_pts = std::move(other.m_pts);
	m_dts = std::move(other.m_dts);
	m_duration = std::move(other.m_duration);
	m_keyFrame = other.m_keyFrame;
	m_attachments = std::move(other.m_attachments);
	m_engine = std::move(other.m_engine);
	other.BecomeEmpty();
	return *this;
}

void Packet::BecomeEmpty() noexcept {
	Item::operator=(Item(-1, Type::Unknown, Kind::Packet, Producer::Demux));
	m_payload = StormByte::Buffer::FIFO{};
	m_pts.reset();
	m_dts.reset();
	m_duration.reset();
	m_keyFrame = false;
	m_attachments.clear();
	m_attachments.shrink_to_fit();
	m_engine.reset();
}

const std::optional<StormByte::Multimedia::Property::Duration>& Packet::Pts() const noexcept {
	return m_pts;
}

const std::optional<StormByte::Multimedia::Property::Duration>& Packet::Dts() const noexcept {
	return m_dts;
}

const std::optional<StormByte::Multimedia::Property::Duration>& Packet::Duration() const noexcept {
	return m_duration;
}

bool Packet::KeyFrame() const noexcept {
	return m_keyFrame;
}

const StormByte::Buffer::FIFO& Packet::Payload() const noexcept {
	return m_payload;
}

StormByte::Buffer::FIFO& Packet::Payload() noexcept {
	return m_payload;
}

const std::vector<SideData>& Packet::Attachments() const noexcept {
	return m_attachments;
}

std::vector<SideData>& Packet::Attachments() noexcept {
	return m_attachments;
}

void Packet::Bind(std::unique_ptr<Engine::Packet::Engine> engine) noexcept {
	m_engine = std::move(engine);
}
