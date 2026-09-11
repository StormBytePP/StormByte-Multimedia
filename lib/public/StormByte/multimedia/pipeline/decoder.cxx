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
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/engine.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>

#include <StormByte/multimedia/name_thread.hxx>

#include <utility>

using namespace StormByte::Multimedia::Pipeline;

Decoder::Decoder(int track, DecoderFlags flags) noexcept
: Step(Kinds{Kind::Packet}, Kinds{Kind::Frame}), m_index(track), m_flags(flags) {
	Launch();
}

Decoder::~Decoder() noexcept = default;

Decoder::operator bool() const noexcept {
	return !Failed() && m_engine && m_engine->IsOpen();
}

int Decoder::Index() const noexcept {
	return m_index;
}

const DecoderFlags& Decoder::Flags() const noexcept {
	return m_flags;
}

void Decoder::Flags(DecoderFlags flags) noexcept {
	m_flags = flags;
}

const std::optional<std::string>& Decoder::Language() const noexcept {
	return m_language;
}

void Decoder::Language(std::string language) noexcept {
	if (language.empty())
		m_language.reset();
	else
		m_language = std::move(language);
}

const std::optional<std::string>& Decoder::Title() const noexcept {
	return m_title;
}

void Decoder::Title(std::string title) noexcept {
	if (title.empty())
		m_title.reset();
	else
		m_title = std::move(title);
}

const std::optional<std::string>& Decoder::Implementation() const noexcept {
	return m_implementation;
}

void Decoder::Implementation(std::string name) noexcept {
	if (name.empty())
		m_implementation.reset();
	else
		m_implementation = std::move(name);
}

const StormByte::Multimedia::Features& Decoder::Require() const noexcept {
	return m_require;
}

void Decoder::Require(StormByte::Multimedia::Features features) noexcept {
	m_require = features;
}

const StormByte::Multimedia::Features& Decoder::Capabilities() const noexcept {
	return m_capabilities;
}

void Decoder::Fail(std::string reason) noexcept {
	m_capabilities = StormByte::Multimedia::Features{};
	// m_engine.reset();
	Step::Fail(std::move(reason));
}

void Decoder::Bind(std::unique_ptr<Engine::Decoder::Engine> engine) noexcept {
	m_engine = std::move(engine);
	m_capabilities = StormByte::Multimedia::Features{};
}

void Decoder::Open() noexcept {}

void Decoder::Work(std::shared_ptr<Item> item) noexcept {
	NameThread("STMM:Decode:" + std::to_string(m_index));
	if (Failed())
		return;
	if (!m_engine) {
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

	while (!m_engine->Send(*this, packet)) {
		if (Failed())
			return;
		std::shared_ptr<Frame> frame = m_engine->Receive(*this);
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
		std::shared_ptr<Frame> frame = m_engine->Receive(*this);
		if (!frame)
			break;
		m_out->Push(frame);
	}
}

void Decoder::Finish() noexcept {
	if (Failed() || !m_engine)
		return;
	m_engine->Flush(*this);
	for (;;) {
		if (Failed())
			return;
		std::shared_ptr<Frame> frame = m_engine->Receive(*this);
		if (!frame)
			return;
		m_out->Push(frame);
	}
}
