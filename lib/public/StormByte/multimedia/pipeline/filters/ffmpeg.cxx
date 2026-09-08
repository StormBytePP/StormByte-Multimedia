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

#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>
#include <StormByte/multimedia/pipeline/engine/packet/engine.hxx>
#include <StormByte/multimedia/type.hxx>

#include <cassert>
#include <utility>

using StormByte::Multimedia::Pipeline::Filter::Analytics;
using StormByte::Multimedia::Pipeline::Filter::FFmpeg;
using StormByte::Multimedia::Pipeline::Filter::Origin;
using StormByte::Multimedia::Pipeline::Filter::Packet;
using StormByte::Multimedia::Pipeline::Filter::Process;
using StormByte::Multimedia::ToString;

FFmpeg::FFmpeg(std::string name) noexcept
: m_name(std::move(name)) {}

FFmpeg::~FFmpeg() noexcept = default;

std::string FFmpeg::Name() const noexcept {
	return std::string(ToString(Media())) + "/" + m_name;
}

void FFmpeg::Reset() noexcept {
	m_failed = false;
	m_reason.clear();
	Clean();
	Setup();
}

void FFmpeg::Call(Pipeline::Frame& frame, Origin origin) noexcept {
	if (Failed())
		return;
	if (frame.Type() != Media())
		return;
	Process(frame, origin);
}

void FFmpeg::Call(StormByte::Multimedia::Pipeline::Packet& packet, Origin origin) noexcept {
	if (Failed())
		return;
	if (packet.Type() != Media())
		return;
	Process(packet, origin);
}

void FFmpeg::Eof(Origin) noexcept {}

void FFmpeg::Flush(Origin) noexcept {}

class StormByte::Multimedia::Pipeline::Filter::Report FFmpeg::Report() const noexcept {
	return {};
}

bool FFmpeg::Failed() const noexcept {
	return m_failed;
}

std::string FFmpeg::ErrorStr() const noexcept {
	assert(Failed());
	if (!Failed())
		return {};
	return "Plugin " + Name() + " failed: " + m_reason;
}

void FFmpeg::Fail(std::string reason) noexcept {
	m_failed = true;
	m_reason = std::move(reason);
}

void FFmpeg::Process(StormByte::Multimedia::Pipeline::Packet&, Origin) noexcept {}

::AVFrame* FFmpeg::Native(Pipeline::Frame& frame) noexcept {
	return frame.m_engine ? frame.m_engine->m_backend.Get() : nullptr;
}

const ::AVFrame* FFmpeg::Native(const Pipeline::Frame& frame) noexcept {
	return frame.m_engine ? frame.m_engine->m_backend.Get() : nullptr;
}

::AVPacket* FFmpeg::Native(StormByte::Multimedia::Pipeline::Packet& packet) noexcept {
	return packet.m_engine ? packet.m_engine->m_backend.Get() : nullptr;
}

const ::AVPacket* FFmpeg::Native(const StormByte::Multimedia::Pipeline::Packet& packet) noexcept {
	return packet.m_engine ? packet.m_engine->m_backend.Get() : nullptr;
}

void FFmpeg::Replace(Pipeline::Frame& frame, ::AVFrame* raw) noexcept {
	if (!frame.m_engine)
		frame.m_engine = std::make_unique<Pipeline::Engine::Frame::Engine>();
	frame.m_engine->m_backend.Free();
	frame.m_engine->m_backend.m_ptr = raw;
	frame.m_engine->m_payloadReady = false;
	frame.m_engine->BindProperties(frame);
}

void FFmpeg::Replace(StormByte::Multimedia::Pipeline::Packet& packet, ::AVPacket* raw) noexcept {
	if (!packet.m_engine)
		packet.m_engine = std::make_unique<Pipeline::Engine::Packet::Engine>();
	packet.m_engine->m_backend.Free();
	packet.m_engine->m_backend.m_ptr = raw;
	packet.m_engine->BindProperties(packet);
}

Process::Process(std::string name) noexcept
: FFmpeg(std::move(name)) {}

void Process::Call(Pipeline::Frame& frame, Origin origin) noexcept {
	if (origin != Origin::Decoder)
		return;
	FFmpeg::Call(frame, origin);
}

void Process::Call(StormByte::Multimedia::Pipeline::Packet&, Origin) noexcept {}

Packet::Packet(std::string name) noexcept
: FFmpeg(std::move(name)) {}

void Packet::Call(Pipeline::Frame&, Origin) noexcept {}

void Packet::Call(StormByte::Multimedia::Pipeline::Packet& packet, Origin origin) noexcept {
	if (origin != Origin::Demux && origin != Origin::Mux)
		return;
	FFmpeg::Call(packet, origin);
}

void Packet::Process(Pipeline::Frame&, Origin) noexcept {}

Analytics::Analytics(std::string name) noexcept
: FFmpeg(std::move(name)) {}

void Analytics::Call(Pipeline::Frame& frame, Origin origin) noexcept {
	if (origin != Origin::Decoder && origin != Origin::Encoder)
		return;
	FFmpeg::Call(frame, origin);
}

void Analytics::Call(StormByte::Multimedia::Pipeline::Packet&, Origin) noexcept {}
