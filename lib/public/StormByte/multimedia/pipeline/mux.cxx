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

#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/pipeline/copy.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/engine/mux/details/container.hxx>
#include <StormByte/multimedia/pipeline/mux.hxx>

namespace StormByte::Multimedia::Pipeline {
	Mux::Mux(const StormByte::Multimedia::Container& container) noexcept
	: m_container(&container), m_engine(std::make_unique<Engine::Mux::Details::Container>()), m_failed(false) {
		if (!container.HasAccess(Access{Operation::Write}))
			Fail("container does not allow write");
	}

	Mux::Mux(Mux&& other) noexcept
	: m_container(other.m_container), m_engine(std::move(other.m_engine)), m_pipe(std::move(other.m_pipe)),
	m_failed(other.m_failed), m_error(std::move(other.m_error)) {
		other.m_failed = true;
	}

	Mux::~Mux() noexcept {
		Finish();
	}

	Mux& Mux::operator=(Mux&& other) noexcept {
		if (this == &other)
			return *this;
		Finish();
		m_container = other.m_container;
		m_engine = std::move(other.m_engine);
		m_pipe = std::move(other.m_pipe);
		m_failed = other.m_failed;
		m_error = std::move(other.m_error);
		other.m_failed = true;
		return *this;
	}

	Mux::operator bool() const noexcept {
		return !m_failed && m_engine && m_engine->IsOpen();
	}

	const StormByte::Multimedia::Container& Mux::Destination() const noexcept {
		return *m_container;
	}

	bool Mux::Failed() const noexcept {
		return m_failed;
	}

	const std::optional<std::string>& Mux::Error() const noexcept {
		return m_error;
	}

	Filter::Chain::Packet& Mux::Pipe() noexcept {
		return m_pipe;
	}

	const Filter::Chain::Packet& Mux::Pipe() const noexcept {
		return m_pipe;
	}

	void Mux::Fail(std::string reason) noexcept {
		m_failed = true;
		m_error = std::move(reason);
		if (m_engine)
			m_engine->Close();
	}

	void Mux::Flush() noexcept {
		if (m_failed || !m_engine)
			return;
		m_engine->Flush(*this);
	}

	void Mux::Finish() noexcept {
		if (!m_engine)
			return;
		if (!m_failed)
			Flush();
		m_engine->Close();
	}

	Encoder& operator>>(Encoder& encoder, Mux& mux) noexcept {
		if (mux.m_failed || encoder.Failed())
			return encoder;
		if (!mux.m_engine) {
			mux.Fail("muxer is not open");
			return encoder;
		}
		mux.m_engine->ReserveEncoder(mux, encoder);
		return encoder;
	}

	Mux& operator>>(Mux& mux, const std::filesystem::path& path) noexcept {
		if (mux.m_failed)
			return mux;
		if (!mux.m_engine) {
			mux.Fail("muxer has no backend");
			return mux;
		}
		mux.m_engine->BindPath(mux, path);
		return mux;
	}

	Mux& operator>>(const StormByte::Multimedia::File& file, Mux& mux) noexcept {
		if (mux.m_failed)
			return mux;
		if (!mux.m_engine) {
			mux.Fail("muxer has no backend");
			return mux;
		}
		mux.m_engine->BindAttachments(mux, file);
		return mux;
	}

	Mux& operator>>(Demux& demux, Mux& mux) noexcept {
		if (mux.m_failed)
			return mux;
		if (!demux.m_file) {
			mux.Fail("demuxer has no source file for attachments");
			return mux;
		}
		return operator>>(*demux.m_file, mux);
	}

	class Packet& operator>>(class Packet& packet, Mux& mux) noexcept {
		if (mux.m_failed)
			return packet;
		if (!mux.m_engine) {
			mux.Fail("muxer has no backend");
			return packet;
		}
		if (!mux.m_pipe.Push(packet)) {
			mux.Fail(mux.m_pipe.Error().value_or("mux packet pipe failed"));
			return packet;
		}
		mux.m_engine->Push(mux, packet);
		return packet;
	}
}
