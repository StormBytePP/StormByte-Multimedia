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

#include <StormByte/multimedia/backend/ffmpeg/AVStream.hxx>
#include <StormByte/multimedia/pipeline/copy.hxx>
#include <StormByte/multimedia/pipeline/copy_impl.hxx>
#include <StormByte/multimedia/pipeline/demux_impl.hxx>
#include <StormByte/multimedia/pipeline/mux_impl.hxx>

namespace StormByte::Multimedia::Pipeline {
	Copy::Copy(int output_index, int input_index) noexcept
	: m_index(output_index), m_input(input_index), m_impl(std::make_unique<Impl>()), m_failed(false) {
		if (m_index < 0)
			Fail("copy output index is invalid");
		else if (m_input < 0)
			Fail("copy input index is invalid");
	}

	Copy::Copy(Copy&&) noexcept = default;
	Copy::~Copy() noexcept = default;
	Copy& Copy::operator=(Copy&&) noexcept = default;

	Copy::operator bool() const noexcept {
		return !m_failed && m_impl && m_impl->bound;
	}

	int Copy::Index() const noexcept {
		return m_index;
	}

	int Copy::InputIndex() const noexcept {
		return m_input;
	}

	bool Copy::Failed() const noexcept {
		return m_failed;
	}

	const std::optional<std::string>& Copy::Error() const noexcept {
		return m_error;
	}

	void Copy::Fail(std::string reason) noexcept {
		m_failed = true;
		m_error = std::move(reason);
	}

	Copy& operator>>(Demux& demux, Copy& copy) noexcept {
		if (copy.m_failed || demux.Failed())
			return copy;
		if (!demux.m_impl) {
			copy.Fail("demuxer is not open");
			return copy;
		}
		if (!copy.m_impl) {
			copy.Fail("copy has no backend");
			return copy;
		}
		if (copy.m_impl->bound) {
			copy.Fail("copy input is already bound");
			return copy;
		}

		for (const auto& stream : demux.m_impl->m_ctx.Streams()) {
			if (stream.Index() != copy.m_input)
				continue;
			copy.m_impl->params = stream.CodecParameters();
			copy.m_impl->timeBase = stream.TimeBase();
			if (const char* language = stream.Tag("language"))
				copy.m_impl->language = language;
			if (const char* title = stream.Tag("title"))
				copy.m_impl->title = title;
			if (!copy.m_impl->params.Get()) {
				copy.Fail("copy stream has no codec parameters");
				return copy;
			}
			copy.m_impl->bound = true;
			return copy;
		}
		copy.Fail("demux has no stream for copy input");
		return copy;
	}

	Copy& operator>>(Copy& copy, Mux& mux) noexcept {
		if (mux.Failed() || copy.m_failed)
			return copy;
		if (!mux.m_impl) {
			mux.Fail("muxer is not open");
			return copy;
		}
		if (mux.m_impl->m_header) {
			mux.Fail("cannot add a track after the header");
			return copy;
		}
		if (!copy.m_impl || !copy.m_impl->bound) {
			copy.Fail("copy input is not bound");
			return copy;
		}
		if (copy.m_index < 0) {
			mux.Fail("copy output index is invalid");
			return copy;
		}
		if (mux.m_impl->m_tracks.contains(copy.m_index)) {
			mux.Fail("duplicate mux output index");
			return copy;
		}
		if (mux.m_impl->m_inToOut.contains(copy.m_input)) {
			mux.Fail("duplicate mux copy input index");
			return copy;
		}

		Mux::Impl::Track track;
		track.copy = &copy;
		track.inIndex = copy.m_input;
		track.timeBase = copy.m_impl->timeBase;
		track.language = copy.m_impl->language;
		track.title = copy.m_impl->title;
		mux.m_impl->m_tracks.emplace(copy.m_index, track);
		mux.m_impl->m_inToOut.emplace(copy.m_input, copy.m_index);
		return copy;
	}
}
