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

#include <StormByte/multimedia/backend/pipeline/decoder.hxx>
#include <StormByte/multimedia/backend/pipeline/frame.hxx>
#include <StormByte/multimedia/buffer/sink.hxx>
#include <StormByte/multimedia/name_thread.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/demuxer.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>

#include <utility>

using namespace StormByte::Multimedia;
using namespace StormByte::Multimedia::Pipeline;

Decoder::Decoder(int track, DecoderFlags flags) noexcept
: Step(Kinds{Kind::Packet}, Kinds{Kind::Frame}),
	m_index(track), m_flags(flags) {
	Launch();
}

Decoder::~Decoder() noexcept {
	Halt();
}

Decoder::operator bool() const noexcept {
	return !Failed() && Ready() && m_backend && m_backend->IsOpen();
}

void Decoder::Implementation(std::string name) noexcept {
	if (name.empty())
		m_implementation.reset();
	else
		m_implementation = std::move(name);
}

void Decoder::Attach(Frame& frame, std::unique_ptr<Backend::Pipeline::Frame> backend) noexcept {
	frame.m_language = m_language;
	frame.m_title = m_title;
	frame.Bind(std::move(backend));
}

void Decoder::CloseCue(Frame& frame, Property::Duration duration) noexcept {
	frame.m_duration = std::move(duration);
}

void Decoder::Bind(std::unique_ptr<Backend::Pipeline::Decoder> backend) noexcept {
	m_backend = std::move(backend);
	m_capabilities = Features{};
}

void Decoder::Stamp(std::optional<std::string> language, std::optional<std::string> title) noexcept {
	if (!language || language->empty())
		m_language.reset();
	else
		m_language = std::move(*language);
	if (!title || title->empty())
		m_title.reset();
	else
		m_title = std::move(*title);
}

void Decoder::AttachOrigin(Demuxer& demuxer) noexcept {
	m_origin = &demuxer;
	Wake().notify_all();
}

void Decoder::Open() noexcept {
	NameThread("STMM:Decode:" + std::to_string(m_index));
	while (!Stopping() && m_origin == nullptr)
		Wait();
	if (Stopping())
		return;
	if (!m_origin) {
		Fail("decoder has no demuxer");
		return;
	}
	while (!Stopping() && !m_origin->Failed() && !m_origin->Ready())
		Wait();
	if (Stopping())
		return;
	if (m_origin->Failed() || !m_origin->Ready()) {
		Fail(m_origin->Error().value_or("demuxer failed"));
		return;
	}

	auto backend = m_origin->OpenDecoder(*this);
	if (!backend)
		return;
	Bind(std::move(backend));
	Step::Open();
}

void Decoder::Work(std::shared_ptr<Item> item) noexcept {
	NameThread("STMM:Decode:" + std::to_string(m_index));
	if (Failed())
		return;
	if (!m_backend) {
		Fail("decoder is not open");
		return;
	}
	auto packet = std::dynamic_pointer_cast<Packet>(item);
	if (!packet) {
		Fail("decoder expected a packet");
		return;
	}
	if (packet->Track() != m_index)
		return;

	while (!m_backend->Send(*this, packet)) {
		if (Failed())
			return;
		std::shared_ptr<Frame> frame = m_backend->Receive(*this);
		if (Failed())
			return;
		if (!frame) {
			Wait();
			continue;
		}
		m_out->Push(frame);
	}

	for (;;) {
		if (Failed())
			return;
		std::shared_ptr<Frame> frame = m_backend->Receive(*this);
		if (!frame)
			break;
		m_out->Push(frame);
	}
}

void Decoder::Finish() noexcept {
	if (Failed() || !m_backend)
		return;
	m_backend->Flush(*this);
	for (;;) {
		if (Failed())
			return;
		std::shared_ptr<Frame> frame = m_backend->Receive(*this);
		if (!frame)
			return;
		m_out->Push(frame);
	}
}
