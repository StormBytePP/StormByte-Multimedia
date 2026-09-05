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

#include <StormByte/multimedia/engine/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVStream.hxx>

using namespace StormByte::Multimedia::Engine::Backend;

FFmpeg::AVStream::AVStream(::AVStream* stream) noexcept
:m_stream(stream) {}

bool FFmpeg::AVStream::operator<(const AVStream& other) const noexcept {
	return Index() < other.Index();
}

int FFmpeg::AVStream::Index() const noexcept {
	return m_stream ? m_stream->index : -1;
}

int FFmpeg::AVStream::Type() const noexcept {
	return m_stream ? m_stream->codecpar->codec_type : AVMEDIA_TYPE_UNKNOWN;
}

FFmpeg::AVCodecParameters FFmpeg::AVStream::CodecParameters() const noexcept {
	return m_stream ? AVCodecParameters(m_stream->codecpar) : AVCodecParameters(nullptr);
}

AVRational FFmpeg::AVStream::TimeBase() const noexcept {
	return m_stream ? m_stream->time_base : AVRational{0, 1};
}

double FFmpeg::AVStream::FrameRate() const noexcept {
	if (!m_stream)
		return 0.0;
	if (m_stream->avg_frame_rate.num && m_stream->avg_frame_rate.den)
		return av_q2d(m_stream->avg_frame_rate);
	if (m_stream->r_frame_rate.num && m_stream->r_frame_rate.den)
		return av_q2d(m_stream->r_frame_rate);
	return 0.0;
}

// oldcode — Metadata
// StormByte::Multimedia::Metadata FFmpeg::AVStream::Metadata() const noexcept {
// 	StormByte::Multimedia::Metadata metadata;
//
// 	if (m_stream && m_stream->metadata) {
// 		AVDictionaryEntry* tag = nullptr;
// 		while ((tag = av_dict_get(m_stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
// 			metadata[tag->key] = tag->value;
// 		}
// 	}
//
// 	return metadata;
// }
