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

#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/engine/decoder/engine.hxx>

using namespace StormByte::Multimedia::Pipeline;
using StormByte::Multimedia::Pipeline::Filter::Origin;

namespace {
	std::shared_ptr<Filter::Chain> Alias(Filter::Chain& pipe) noexcept {
		return std::shared_ptr<Filter::Chain>(&pipe, [](Filter::Chain*) {});
	}
}

Decoder::Decoder(int stream_index, DecoderFlags flags,
	std::shared_ptr<Filter::Chain> pipe) noexcept
: m_index(stream_index), m_flags(flags), m_pipe(std::move(pipe)), m_failed(false) {}

Decoder::Decoder(int stream_index, DecoderFlags flags, Filter::Chain& pipe) noexcept
: Decoder(stream_index, flags, Alias(pipe)) {}

Decoder::Decoder(Decoder&&) noexcept = default;
Decoder::~Decoder() noexcept = default;
Decoder& Decoder::operator=(Decoder&&) noexcept = default;

Decoder::operator bool() const noexcept {
	return !m_failed && m_engine && m_engine->IsOpen();
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

std::shared_ptr<Filter::Chain>& Decoder::Pipe() noexcept {
	return m_pipe;
}

const std::shared_ptr<Filter::Chain>& Decoder::Pipe() const noexcept {
	return m_pipe;
}

void Decoder::Pipe(std::shared_ptr<Filter::Chain> pipe) noexcept {
	m_pipe = std::move(pipe);
}

void Decoder::Pipe(Filter::Chain& pipe) noexcept {
	m_pipe = Alias(pipe);
}

bool Decoder::Failed() const noexcept {
	return m_failed;
}

const std::optional<std::string>& Decoder::Error() const noexcept {
	return m_error;
}

void Decoder::Fail(std::string reason) noexcept {
	m_failed = true;
	m_error = std::move(reason);
	m_capabilities = StormByte::Multimedia::Features{};
	m_engine.reset();
}

void Decoder::Bind(std::unique_ptr<Engine::Decoder::Engine> engine) noexcept {
	m_engine = std::move(engine);
	m_failed = false;
	m_error.reset();
}

void Decoder::Flush() noexcept {
	if (m_failed || !m_engine)
		return;
	m_engine->Flush(*this);
	if (m_pipe) {
		m_pipe->Eof(Origin::Decoder);
		if (m_pipe->Failed())
			Fail(m_pipe->ErrorStr());
	}
}

Packet& StormByte::Multimedia::Pipeline::operator>>(Packet& packet, Decoder& decoder) noexcept {
	if (decoder.m_failed || !decoder.m_engine)
		return packet;
	if (packet.StreamIndex() != decoder.m_index)
		return packet;
	decoder.m_engine->Send(decoder, packet);
	return packet;
}

Decoder& StormByte::Multimedia::Pipeline::operator>>(Decoder& decoder, Frame& frame) noexcept {
	if (decoder.m_failed || !decoder.m_engine)
		return decoder;
	if (!decoder.m_engine->Receive(decoder, frame))
		return decoder;
	if (decoder.m_failed)
		return decoder;

	if (decoder.m_pipe) {
		decoder.m_pipe->Call(frame, Origin::Decoder);
		if (decoder.m_pipe->Failed()) {
			decoder.Fail(decoder.m_pipe->ErrorStr());
			frame = Frame{};
			return decoder;
		}
	}
	return decoder;
}
