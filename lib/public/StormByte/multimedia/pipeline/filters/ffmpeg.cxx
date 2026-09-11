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
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>
#include <StormByte/multimedia/pipeline/engine/packet/engine.hxx>
#include <StormByte/multimedia/pipeline/filters/ffmpeg.hxx>
#include <StormByte/multimedia/type.hxx>

#include <StormByte/multimedia/name_thread.hxx>

#include <limits>
#include <utility>

using StormByte::Multimedia::Pipeline::Filter::Analytics;
using StormByte::Multimedia::Pipeline::Filter::FFmpeg;
using StormByte::Multimedia::Pipeline::Filter::Packet;
using StormByte::Multimedia::Pipeline::Filter::Process;
using StormByte::Multimedia::Pipeline::Kind;
using StormByte::Multimedia::Pipeline::Kinds;
using StormByte::Multimedia::ToString;

FFmpeg::FFmpeg(std::string name, Kinds receives, Kinds produces) noexcept
: Step(receives, produces), m_name(std::move(name)), m_hold(0), m_heldFor(0) {}

FFmpeg::~FFmpeg() noexcept = default;

std::string FFmpeg::Name() const noexcept {
	return std::string(ToString(Media())) + "/" + m_name;
}

void FFmpeg::Process(const Pipeline::Frame&) noexcept {}

void FFmpeg::Process(const Pipeline::Packet&) noexcept {}

class StormByte::Multimedia::Pipeline::Filter::Report FFmpeg::Report() const noexcept {
	return {};
}

void FFmpeg::Fail(std::string reason) noexcept {
	m_hold = 0;
	m_heldFor = 0;
	m_queue.clear();
	Pipeline::Step::Fail(std::move(reason));
}

void FFmpeg::Hold(std::uint8_t n) noexcept {
	if (Held()) {
		Fail("Hold while already Held");
		return;
	}
	if (!m_current) {
		Fail("Hold without a unit");
		return;
	}
	m_hold = n == 0 ? std::numeric_limits<std::uint8_t>::max() : n;
	m_heldFor = 0;
	Park();
}

void FFmpeg::Release() noexcept {
	if (!Held())
		return;
	m_hold = 0;
	m_heldFor = 0;
	auto parked = std::move(m_queue);
	for (auto& item : parked) {
		m_current = item;
		if (m_current->Kind() == Pipeline::Kind::Frame)
			Process(static_cast<const Pipeline::Frame&>(*m_current));
		else
			Process(static_cast<const Pipeline::Packet&>(*m_current));
		if (Failed())
			return;
		m_out->Push(std::move(m_current));
	}
	m_current.reset();
}

bool FFmpeg::Held() const noexcept {
	return m_hold > 0;
}

std::uint8_t FFmpeg::HeldFor() const noexcept {
	return m_heldFor;
}

void FFmpeg::Eof() noexcept {}

::AVFrame* FFmpeg::AVFrame() noexcept {
	auto frame = std::dynamic_pointer_cast<Pipeline::Frame>(m_current);
	if (!frame || !frame->m_engine)
		return nullptr;
	return frame->m_engine->m_backend.Get();
}

::AVPacket* FFmpeg::AVPacket() noexcept {
	auto packet = std::dynamic_pointer_cast<Pipeline::Packet>(m_current);
	if (!packet || !packet->m_engine)
		return nullptr;
	return packet->m_engine->m_backend.Get();
}

void FFmpeg::Save(::AVFrame* raw) noexcept {
	auto frame = std::dynamic_pointer_cast<Pipeline::Frame>(m_current);
	if (!frame)
		return;
	if (!frame->m_engine)
		frame->m_engine = std::make_unique<Pipeline::Engine::Frame::Engine>();
	frame->m_engine->m_backend.Free();
	frame->m_engine->m_backend.m_ptr = raw;
	frame->m_engine->m_payloadReady = false;
	frame->m_engine->BindProperties(*frame);
}

void FFmpeg::Save(::AVPacket* raw) noexcept {
	auto packet = std::dynamic_pointer_cast<Pipeline::Packet>(m_current);
	if (!packet)
		return;
	if (!packet->m_engine)
		packet->m_engine = std::make_unique<Pipeline::Engine::Packet::Engine>();
	packet->m_engine->m_backend.Free();
	packet->m_engine->m_backend.m_ptr = raw;
	packet->m_engine->BindProperties(*packet);
}

void FFmpeg::Open() noexcept {
	NameThread("STMM:FFmpeg:" + m_name);
	Clean();
	Setup();
}

void FFmpeg::LastChance(const Pipeline::Frame&) noexcept {}

void FFmpeg::LastChance(const Pipeline::Packet&) noexcept {}

void FFmpeg::Park() noexcept {
	if (!m_current)
		return;
	if (!m_queue.empty() && m_queue.back() == m_current)
		return;
	if (m_heldFor >= m_hold) {
		Fail("Hold exceeded");
		return;
	}
	m_queue.push_back(m_current);
	++m_heldFor;
}

void FFmpeg::CallLastChance() noexcept {
	if (!m_current)
		return;
	if (m_current->Kind() == Pipeline::Kind::Frame)
		LastChance(static_cast<const Pipeline::Frame&>(*m_current));
	else
		LastChance(static_cast<const Pipeline::Packet&>(*m_current));
}

void FFmpeg::Work(std::shared_ptr<Pipeline::Item> item) noexcept {
	m_current = std::move(item);
	if (m_current->Kind() == Pipeline::Kind::Frame)
		Process(static_cast<const Pipeline::Frame&>(*m_current));
	else
		Process(static_cast<const Pipeline::Packet&>(*m_current));
	if (Failed())
		return;
	if (Held()) {
		if (m_heldFor >= m_hold) {
			CallLastChance();
			if (Held()) {
				Fail("Hold exceeded");
				return;
			}
		}
		else {
			Park();
			return;
		}
	}
	if (m_current)
		m_out->Push(std::move(m_current));
}

void FFmpeg::Finish() noexcept {
	if (Held()) {
		if (!m_current && !m_queue.empty())
			m_current = m_queue.back();
		CallLastChance();
		if (Held())
			Fail("Hold + EoF without Release");
	}
	if (!Failed())
		Eof();
}

Process::Process(std::string name) noexcept
: FFmpeg(std::move(name), Kinds{Kind::Frame}, Kinds{Kind::Frame}) {}

Process::Process(std::string name, Kinds receives, Kinds produces) noexcept
: FFmpeg(std::move(name), receives, produces) {}

Packet::Packet(std::string name) noexcept
: FFmpeg(std::move(name), Kinds{Kind::Packet}, Kinds{Kind::Packet}) {}

Packet::Packet(std::string name, Kinds receives, Kinds produces) noexcept
: FFmpeg(std::move(name), receives, produces) {}

Analytics::Analytics(std::string name) noexcept
: FFmpeg(std::move(name), Kinds{Kind::Frame}, Kinds{Kind::Frame}) {}

Analytics::Analytics(std::string name, Kinds receives, Kinds produces) noexcept
: FFmpeg(std::move(name), receives, produces) {}
