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

#include <StormByte/multimedia/pipeline/frame.hxx>
#include <StormByte/multimedia/pipeline/frame_impl.hxx>

using namespace StormByte::Multimedia::Pipeline;

Frame::Frame() noexcept
: m_streamIndex(-1) {}

Frame::Frame(int stream_index, StormByte::Buffer::FIFO payload,
	std::optional<StormByte::Multimedia::Property::Duration> pts,
	std::optional<StormByte::Multimedia::Property::Duration> duration,
	std::optional<StormByte::Multimedia::Property::Video> video,
	std::vector<class SideData> attachments) noexcept
: m_streamIndex(stream_index), m_payload(std::move(payload)),
m_pts(std::move(pts)), m_duration(std::move(duration)),
m_video(std::move(video)), m_attachments(std::move(attachments)) {}

Frame::Frame(Frame&&) noexcept = default;
Frame::~Frame() noexcept = default;
Frame& Frame::operator=(Frame&&) noexcept = default;

int Frame::StreamIndex() const noexcept {
	return m_streamIndex;
}

const std::optional<StormByte::Multimedia::Property::Duration>& Frame::Pts() const noexcept {
	return m_pts;
}

const std::optional<StormByte::Multimedia::Property::Duration>& Frame::Duration() const noexcept {
	return m_duration;
}

const std::optional<StormByte::Multimedia::Property::Video>& Frame::Video() const noexcept {
	return m_video;
}

const std::vector<class SideData>& Frame::Attachments() const noexcept {
	return m_attachments;
}

StormByte::Buffer::FIFO& Frame::Payload() noexcept {
	if (m_impl && !m_impl->m_payloadReady) {
		StormByte::Buffer::DataType bytes;
		m_impl->m_backend.CopyPrimaryBuffer(bytes);
		m_payload = StormByte::Buffer::FIFO{std::move(bytes)};
		m_impl->m_payloadReady = true;
	}
	return m_payload;
}

const StormByte::Buffer::FIFO& Frame::Payload() const noexcept {
	return m_payload;
}

void Frame::Bind(std::unique_ptr<Impl> impl) noexcept {
	m_impl = std::move(impl);
}
