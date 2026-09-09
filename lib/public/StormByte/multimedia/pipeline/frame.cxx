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

#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>
#include <StormByte/multimedia/pipeline/frame.hxx>

#include <utility>

using namespace StormByte::Multimedia::Pipeline;
using StormByte::Multimedia::Type;

Frame::Frame() noexcept
: Item(-1, Type::Unknown, Kind::Frame, Producer::Decoder) {}

Frame::Frame(int track, enum Type type, enum Producer producer, StormByte::Buffer::FIFO payload,
	std::optional<StormByte::Multimedia::Property::Duration> pts,
	std::optional<StormByte::Multimedia::Property::Duration> duration,
	std::optional<StormByte::Multimedia::Property::Video> video,
	std::vector<class SideData> attachments,
	std::optional<StormByte::Multimedia::Property::Audio> audio) noexcept
: Item(track, type, Kind::Frame, producer),
	m_payload(std::move(payload)),
	m_pts(std::move(pts)), m_duration(std::move(duration)),
	m_video(std::move(video)), m_audio(std::move(audio)),
	m_attachments(std::move(attachments)) {}

Frame::Frame(const Frame& other) noexcept
: Item(other),
	m_payload(other.m_payload),
	m_pts(other.m_pts),
	m_duration(other.m_duration),
	m_video(other.m_video),
	m_audio(other.m_audio),
	m_language(other.m_language),
	m_title(other.m_title),
	m_attachments(other.m_attachments) {
	if (other.m_engine)
		m_engine = std::make_unique<Engine::Frame::Engine>(*other.m_engine);
}

Frame::Frame(Frame&& other) noexcept
: Item(std::move(other)),
	m_payload(std::move(other.m_payload)),
	m_pts(std::move(other.m_pts)),
	m_duration(std::move(other.m_duration)),
	m_video(std::move(other.m_video)),
	m_audio(std::move(other.m_audio)),
	m_language(std::move(other.m_language)),
	m_title(std::move(other.m_title)),
	m_attachments(std::move(other.m_attachments)),
	m_engine(std::move(other.m_engine)) {
	other.BecomeEmpty();
}

Frame::~Frame() noexcept = default;

Frame& Frame::operator=(const Frame& other) noexcept {
	if (this == &other)
		return *this;
	Item::operator=(other);
	m_payload = other.m_payload;
	m_pts = other.m_pts;
	m_duration = other.m_duration;
	m_video = other.m_video;
	m_audio = other.m_audio;
	m_language = other.m_language;
	m_title = other.m_title;
	m_attachments = other.m_attachments;
	if (other.m_engine)
		m_engine = std::make_unique<Engine::Frame::Engine>(*other.m_engine);
	else
		m_engine.reset();
	return *this;
}

Frame& Frame::operator=(Frame&& other) noexcept {
	if (this == &other)
		return *this;
	Item::operator=(std::move(other));
	m_payload = std::move(other.m_payload);
	m_pts = std::move(other.m_pts);
	m_duration = std::move(other.m_duration);
	m_video = std::move(other.m_video);
	m_audio = std::move(other.m_audio);
	m_language = std::move(other.m_language);
	m_title = std::move(other.m_title);
	m_attachments = std::move(other.m_attachments);
	m_engine = std::move(other.m_engine);
	other.BecomeEmpty();
	return *this;
}

void Frame::BecomeEmpty() noexcept {
	Item::operator=(Item(-1, Type::Unknown, Kind::Frame, Producer::Decoder));
	m_payload = StormByte::Buffer::FIFO{};
	m_pts.reset();
	m_duration.reset();
	m_video.reset();
	m_audio.reset();
	m_language.reset();
	m_title.reset();
	m_attachments.clear();
	m_attachments.shrink_to_fit();
	m_engine.reset();
}

const std::optional<StormByte::Multimedia::Property::Duration>& Frame::Pts() const noexcept {
	return m_pts;
}

const std::optional<StormByte::Multimedia::Property::Duration>& Frame::Duration() const noexcept {
	return m_duration;
}

const std::optional<std::string>& Frame::Language() const noexcept {
	return m_language;
}

void Frame::Language(std::string language) noexcept {
	if (language.empty()) {
		m_language.reset();
		return;
	}
	m_language = std::move(language);
}

const std::optional<std::string>& Frame::Title() const noexcept {
	return m_title;
}

void Frame::Title(std::string title) noexcept {
	if (title.empty()) {
		m_title.reset();
		return;
	}
	m_title = std::move(title);
}

const std::optional<StormByte::Multimedia::Property::Video>& Frame::Video() const noexcept {
	return m_video;
}

const std::optional<StormByte::Multimedia::Property::Audio>& Frame::Audio() const noexcept {
	return m_audio;
}

const std::vector<class SideData>& Frame::Attachments() const noexcept {
	return m_attachments;
}

StormByte::Buffer::FIFO& Frame::Payload() noexcept {
	if (m_engine && !m_engine->m_payloadReady) {
		StormByte::Buffer::DataType bytes;
		m_engine->m_backend.CopyPrimaryBuffer(bytes);
		m_payload = StormByte::Buffer::FIFO{std::move(bytes)};
		m_engine->m_payloadReady = true;
	}
	return m_payload;
}

const StormByte::Buffer::FIFO& Frame::Payload() const noexcept {
	return m_payload;
}

void Frame::Bind(std::unique_ptr<Engine::Frame::Engine> engine) noexcept {
	m_engine = std::move(engine);
}
