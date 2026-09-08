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

using StormByte::Multimedia::Pipeline::Filter::Analytics;
using StormByte::Multimedia::Pipeline::Filter::FFmpeg;
using StormByte::Multimedia::Pipeline::Filter::Process;
using StormByte::Multimedia::Pipeline::Filter::Report;
using StormByte::Multimedia::Pipeline::Filter::Role;
using StormByte::Multimedia::Pipeline::Filter::Roles;
using StormByte::Multimedia::Type;
using FilterPacket = StormByte::Multimedia::Pipeline::Filter::Packet;

FFmpeg::~FFmpeg() noexcept = default;

class Report FFmpeg::Report() const noexcept {
	return {};
}

bool FFmpeg::Failed() const noexcept {
	return m_failed;
}

const std::optional<std::string>& FFmpeg::Error() const noexcept {
	return m_error;
}

void FFmpeg::Fail(std::string reason) noexcept {
	m_failed = true;
	m_error = std::move(reason);
}

bool FFmpeg::Gate(const StormByte::Multimedia::Pipeline::Frame& frame, Role role) const noexcept {
	if (m_failed)
		return false;
	if (!Accepts().Has(role))
		return false;
	return frame.Type() == Media();
}

bool FFmpeg::Gate(const class StormByte::Multimedia::Pipeline::Packet& packet, Role role) const noexcept {
	if (m_failed)
		return false;
	if (!Accepts().Has(role))
		return false;
	return packet.Type() == Media();
}

FFmpeg::Frame::Frame(StormByte::Multimedia::Pipeline::Frame& frame) noexcept
: m_view(&frame) {}

FFmpeg::Frame::Frame(std::unique_ptr<StormByte::Multimedia::Pipeline::Frame> frame) noexcept
: m_owned(std::move(frame)) {}

FFmpeg::Frame::Frame(Frame&& other) noexcept
: m_view(other.m_view), m_owned(std::move(other.m_owned)) {
	other.m_view = nullptr;
}

FFmpeg::Frame::~Frame() noexcept = default;

FFmpeg::Frame& FFmpeg::Frame::operator=(Frame&& other) noexcept {
	if (this == &other)
		return *this;
	m_view = other.m_view;
	m_owned = std::move(other.m_owned);
	other.m_view = nullptr;
	return *this;
}

StormByte::Multimedia::Pipeline::Frame& FFmpeg::Frame::Ref() noexcept {
	return m_owned ? *m_owned : *m_view;
}

const StormByte::Multimedia::Pipeline::Frame& FFmpeg::Frame::Ref() const noexcept {
	return m_owned ? *m_owned : *m_view;
}

int FFmpeg::Frame::StreamIndex() const noexcept {
	return Ref().StreamIndex();
}

const std::optional<StormByte::Multimedia::Property::Duration>& FFmpeg::Frame::Pts() const noexcept {
	return Ref().Pts();
}

void FFmpeg::Frame::Pts(std::optional<StormByte::Multimedia::Property::Duration> pts) noexcept {
	FFmpeg::SetPts(Ref(), std::move(pts));
}

const std::optional<StormByte::Multimedia::Property::Duration>& FFmpeg::Frame::Duration() const noexcept {
	return Ref().Duration();
}

void FFmpeg::Frame::Duration(std::optional<StormByte::Multimedia::Property::Duration> duration) noexcept {
	FFmpeg::SetDuration(Ref(), std::move(duration));
}

const std::optional<std::string>& FFmpeg::Frame::Language() const noexcept {
	return Ref().Language();
}

void FFmpeg::Frame::Language(std::string language) noexcept {
	Ref().Language(std::move(language));
}

const std::optional<std::string>& FFmpeg::Frame::Title() const noexcept {
	return Ref().Title();
}

void FFmpeg::Frame::Title(std::string title) noexcept {
	Ref().Title(std::move(title));
}

const std::optional<StormByte::Multimedia::Property::Video>& FFmpeg::Frame::Video() const noexcept {
	return Ref().Video();
}

void FFmpeg::Frame::Video(std::optional<StormByte::Multimedia::Property::Video> video) noexcept {
	FFmpeg::SetVideo(Ref(), std::move(video));
}

const std::optional<StormByte::Multimedia::Property::Audio>& FFmpeg::Frame::Audio() const noexcept {
	return Ref().Audio();
}

void FFmpeg::Frame::Audio(std::optional<StormByte::Multimedia::Property::Audio> audio) noexcept {
	FFmpeg::SetAudio(Ref(), std::move(audio));
}

const std::vector<class StormByte::Multimedia::Pipeline::SideData>& FFmpeg::Frame::Attachments() const noexcept {
	return Ref().Attachments();
}

StormByte::Buffer::FIFO& FFmpeg::Frame::Payload() noexcept {
	return Ref().Payload();
}

const StormByte::Buffer::FIFO& FFmpeg::Frame::Payload() const noexcept {
	return Ref().Payload();
}

FFmpeg::Frame FFmpeg::Clone(const Frame& frame) noexcept {
	return Frame(std::unique_ptr<StormByte::Multimedia::Pipeline::Frame>(
		new StormByte::Multimedia::Pipeline::Frame(frame.Ref()))
	);
}

::AVFrame* FFmpeg::Native(StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
	if (!frame.m_engine)
		return nullptr;
	return frame.m_engine->m_backend.Get();
}

const ::AVFrame* FFmpeg::Native(const StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
	if (!frame.m_engine)
		return nullptr;
	return frame.m_engine->m_backend.Get();
}

::AVFrame* FFmpeg::Native(Frame& frame) noexcept {
	return Native(frame.Ref());
}

const ::AVFrame* FFmpeg::Native(const Frame& frame) noexcept {
	return Native(frame.Ref());
}

::AVPacket* FFmpeg::Native(class StormByte::Multimedia::Pipeline::Packet& packet) noexcept {
	if (!packet.m_engine)
		return nullptr;
	return packet.m_engine->m_backend.Get();
}

const ::AVPacket* FFmpeg::Native(const class StormByte::Multimedia::Pipeline::Packet& packet) noexcept {
	if (!packet.m_engine)
		return nullptr;
	return packet.m_engine->m_backend.Get();
}

void FFmpeg::Replace(StormByte::Multimedia::Pipeline::Frame& frame, ::AVFrame* raw) noexcept {
	if (!raw) {
		Fail("replace: null AVFrame");
		return;
	}
	if (!frame.m_engine)
		frame.m_engine = std::make_unique<StormByte::Multimedia::Pipeline::Engine::Frame::Engine>();
	frame.m_engine->m_backend.Free();
	frame.m_engine->m_backend.m_ptr = raw;
	frame.m_engine->m_payloadReady = false;
	frame.m_payload = StormByte::Buffer::FIFO{};
	frame.m_engine->BindProperties(frame);
}

void FFmpeg::Replace(class StormByte::Multimedia::Pipeline::Packet& packet, ::AVPacket* raw) noexcept {
	if (!raw) {
		Fail("replace: null AVPacket");
		return;
	}
	if (!packet.m_engine)
		packet.m_engine = std::make_unique<StormByte::Multimedia::Pipeline::Engine::Packet::Engine>();
	packet.m_engine->m_backend.Free();
	packet.m_engine->m_backend.m_ptr = raw;
	packet.m_engine->BindProperties(packet);
}

void FFmpeg::SetVideo(StormByte::Multimedia::Pipeline::Frame& frame,
	std::optional<StormByte::Multimedia::Property::Video> video) noexcept {
	frame.m_video = std::move(video);
}

void FFmpeg::SetAudio(StormByte::Multimedia::Pipeline::Frame& frame,
	std::optional<StormByte::Multimedia::Property::Audio> audio) noexcept {
	frame.m_audio = std::move(audio);
}

void FFmpeg::SetPts(StormByte::Multimedia::Pipeline::Frame& frame,
	std::optional<StormByte::Multimedia::Property::Duration> pts) noexcept {
	frame.m_pts = std::move(pts);
}

void FFmpeg::SetDuration(StormByte::Multimedia::Pipeline::Frame& frame,
	std::optional<StormByte::Multimedia::Property::Duration> duration) noexcept {
	frame.m_duration = std::move(duration);
}

void FFmpeg::CopyFrame(StormByte::Multimedia::Pipeline::Frame& dst,
	const StormByte::Multimedia::Pipeline::Frame& src) noexcept {
	dst = src;
}

Roles Process::Accepts() const noexcept {
	return Roles(Role::Process);
}

bool Process::Push(StormByte::Multimedia::Pipeline::Frame& frame, Role role) noexcept {
	if (Failed())
		return false;
	if (Gate(frame, role))
		ProcessFrame(frame);
	return !Failed();
}

Roles FilterPacket::Accepts() const noexcept {
	return Roles(Role::Process);
}

bool FilterPacket::Push(class StormByte::Multimedia::Pipeline::Packet& packet, Role role) noexcept {
	if (Failed())
		return false;
	if (Gate(packet, role))
		ProcessPacket(packet);
	return !Failed();
}

bool Analytics::Push(StormByte::Multimedia::Pipeline::Frame& frame, Role role) noexcept {
	if (Failed())
		return false;
	if (Gate(frame, role))
		Analyze(frame, role);
	return !Failed();
}
