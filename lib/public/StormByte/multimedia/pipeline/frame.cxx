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

using namespace StormByte::Multimedia::Pipeline;
using StormByte::Multimedia::Type;

/**
 * @brief Empty frame.
 */
Frame::Frame() noexcept
: m_type(Type::Unknown), m_streamIndex(-1) {}

/**
 * @brief Builds a frame without a backend buffer.
 */
Frame::Frame(enum Type type, int stream_index, StormByte::Buffer::FIFO payload,
	std::optional<StormByte::Multimedia::Property::Duration> pts,
	std::optional<StormByte::Multimedia::Property::Duration> duration,
	std::optional<StormByte::Multimedia::Property::Video> video,
	std::vector<class SideData> attachments,
	std::optional<StormByte::Multimedia::Property::Audio> audio) noexcept
: m_type(type), m_streamIndex(stream_index), m_payload(std::move(payload)),
m_pts(std::move(pts)), m_duration(std::move(duration)),
m_video(std::move(video)), m_audio(std::move(audio)),
m_attachments(std::move(attachments)) {}

/**
 * @brief Deep copy (metadata, FIFO and cloned AVFrame).
 * @param other Source frame.
 */
Frame::Frame(const Frame& other) noexcept
: m_type(other.m_type),
m_streamIndex(other.m_streamIndex),
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

/**
 * @brief Deep copy assignment.
 * @param other Source frame.
 * @return *this.
 */
Frame& Frame::operator=(const Frame& other) noexcept {
	if (this == &other)
		return *this;
	m_type = other.m_type;
	m_streamIndex = other.m_streamIndex;
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

Frame::Frame(Frame&&) noexcept = default;
Frame::~Frame() noexcept = default;
Frame& Frame::operator=(Frame&&) noexcept = default;

enum Type Frame::Type() const noexcept {
	return m_type;
}

int Frame::StreamIndex() const noexcept {
	return m_streamIndex;
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
