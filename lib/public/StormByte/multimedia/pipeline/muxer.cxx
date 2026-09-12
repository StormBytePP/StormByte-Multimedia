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

#include <StormByte/multimedia/backend/pipeline/detail/muxer/matroska/container.hxx>
#include <StormByte/multimedia/backend/pipeline/demuxer.hxx>
#include <StormByte/multimedia/backend/pipeline/muxer.hxx>
#include <StormByte/multimedia/buffer/sink.hxx>
#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/name_thread.hxx>
#include <StormByte/multimedia/pipeline/demuxer.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/muxer.hxx>
#include <StormByte/multimedia/pipeline/packet.hxx>
#include <StormByte/multimedia/pipeline/remuxer.hxx>
#include <StormByte/multimedia/type.hxx>

#include <cctype>
#include <chrono>
#include <string_view>
#include <utility>

extern "C" {
	#include <libavcodec/codec_par.h>
	#include <libavutil/rational.h>
}

using namespace StormByte::Multimedia;
using namespace StormByte::Multimedia::Pipeline;

namespace {
	bool EqualsIgnoreCase(std::string_view a, std::string_view b) noexcept {
		if (a.size() != b.size())
			return false;
		for (std::size_t i = 0; i < a.size(); ++i) {
			const auto left = static_cast<unsigned char>(a[i]);
			const auto right = static_cast<unsigned char>(b[i]);
			if (std::tolower(left) != std::tolower(right))
				return false;
		}
		return true;
	}

	bool IsMatroskaFamily(std::string_view name) noexcept {
		return EqualsIgnoreCase(name, "matroska")
			|| EqualsIgnoreCase(name, "webm")
			|| EqualsIgnoreCase(name, "mkv");
	}
}

Muxer::Muxer(std::shared_ptr<StormByte::Logger::Log> log,
	const Container& container) noexcept
: Step(std::move(log), Producer::Muxer, Kinds{Kind::Packet}, Kinds{}),
	m_container(&container),
	m_origin(nullptr),
	m_closed(false), m_positionNs(-1) {
	if (!container.HasAccess(Access{Operation::Write})) {
		Fail("container does not allow write");
		return;
	}
	const std::string_view name{container.Name()};
	if (IsMatroskaFamily(name))
		m_backend = std::make_unique<Backend::Pipeline::Detail::Muxer::Matroska::Container>();
	else {
		Fail("no muxer backend for destination container");
		return;
	}
	Launch();
}

Muxer::~Muxer() noexcept {
	Halt();
	if (m_backend)
		m_backend->Close();
}

Muxer::operator bool() const noexcept {
	return !Failed() && !m_closed.load(std::memory_order_acquire) && Ready()
		&& m_backend && m_backend->IsOpen();
}

bool Muxer::Closed() const noexcept {
	return m_closed.load(std::memory_order_acquire) || Failed();
}

std::optional<Property::Duration> Muxer::Position() const noexcept {
	const std::int64_t ns = m_positionNs.load(std::memory_order_acquire);
	if (ns < 0)
		return std::nullopt;
	return Property::Duration{std::chrono::nanoseconds{ns}};
}

const Container& Muxer::Destination() const noexcept {
	return *m_container;
}

std::optional<std::string> Muxer::Language(int output_index) const noexcept {
	const auto it = m_language.find(output_index);
	if (it == m_language.end() || it->second.empty())
		return std::nullopt;
	return it->second;
}

void Muxer::Language(int output_index, std::string language) noexcept {
	if (language.empty())
		m_language.erase(output_index);
	else
		m_language[output_index] = std::move(language);
}

std::optional<std::string> Muxer::Title(int output_index) const noexcept {
	const auto it = m_title.find(output_index);
	if (it == m_title.end() || it->second.empty())
		return std::nullopt;
	return it->second;
}

void Muxer::Title(int output_index, std::string title) noexcept {
	if (title.empty())
		m_title.erase(output_index);
	else
		m_title[output_index] = std::move(title);
}

void Muxer::Open() noexcept {
	if (!m_backend) {
		Fail("muxer has no backend");
		return;
	}
	Step::Open();
}

void Muxer::Work(std::shared_ptr<Item> item) noexcept {
	NameThread("STMM:Muxer");

	if (Failed() || !m_backend)
		return;
	auto packet = std::dynamic_pointer_cast<Packet>(item);
	if (!packet) {
		Fail("muxer expected a packet");
		return;
	}
	while (!m_backend->Push(*this, packet)) {
		if (Failed())
			return;
		Wait();
	}
	const auto type = packet->Type();
	if (type == Type::Video || type == Type::Audio) {
		if (const auto& pts = packet->Pts(); pts) {
			const auto ns = pts->Nanoseconds().count();
			if (type == Type::Video)
				m_positionNs.store(ns, std::memory_order_release);
			else {
				std::int64_t current = m_positionNs.load(std::memory_order_acquire);
				if (current < 0)
					m_positionNs.store(ns, std::memory_order_release);
			}
		}
	}
}

void Muxer::Finish() noexcept {
	if (m_backend && !Failed())
		m_backend->Flush(*this);
	m_closed.store(true, std::memory_order_release);
}

bool Muxer::BindEncoderStream(Encoder& encoder, void* avStream) noexcept {
	return encoder.MuxBindStream(avStream);
}

bool Muxer::RemuxCodec(int inIndex, void*& params, void* timeBase) noexcept {
	params = nullptr;
	if (!timeBase || !m_origin || !m_origin->Ready() || !m_origin->m_backend)
		return false;
	return m_origin->m_backend->CloneStream(
		inIndex,
		*reinterpret_cast<::AVCodecParameters**>(&params),
		*static_cast<AVRational*>(timeBase));
}

Encoder& StormByte::Multimedia::Pipeline::operator>>(Encoder& encoder, Muxer& muxer) noexcept {
	if (!muxer.m_plan)
		muxer.m_plan = encoder.m_plan;
	muxer.m_in->Notify(muxer.Wake());
	if (muxer.Failed() || encoder.Failed())
		return encoder;
	if (!muxer.m_backend) {
		muxer.Fail("muxer has no backend");
		return encoder;
	}
	muxer.m_backend->ReserveEncoder(muxer, encoder);
	encoder.m_out->Bind(encoder.Index(), *muxer.m_in);
	if (const std::size_t cap = muxer.InputCeiling(); cap > 0)
		muxer.m_in->Capacity(encoder.Index(), cap);
	return encoder;
}

Remuxer& StormByte::Multimedia::Pipeline::operator>>(Remuxer& remuxer, Muxer& muxer) noexcept {
	if (!muxer.m_plan)
		muxer.m_plan = remuxer.m_plan;
	muxer.m_in->Notify(muxer.Wake());
	if (muxer.Failed() || remuxer.Failed())
		return remuxer;
	if (!muxer.m_backend) {
		muxer.Fail("muxer has no backend");
		return remuxer;
	}
	if (!muxer.m_backend->ReserveRemux(muxer, remuxer.In()))
		return remuxer;
	remuxer.m_out->Bind(remuxer.In(), *muxer.m_in);
	if (const std::size_t cap = muxer.InputCeiling(); cap > 0)
		muxer.m_in->Capacity(remuxer.In(), cap);
	return remuxer;
}

Muxer& StormByte::Multimedia::Pipeline::operator>>(Muxer& muxer, const std::filesystem::path& path) noexcept {
	if (muxer.Failed())
		return muxer;
	if (!muxer.m_backend) {
		muxer.Fail("muxer has no backend");
		return muxer;
	}
	muxer.m_backend->BindPath(muxer, path);
	return muxer;
}

Muxer& StormByte::Multimedia::Pipeline::operator>>(const File& file, Muxer& muxer) noexcept {
	if (muxer.Failed())
		return muxer;
	if (!muxer.m_backend) {
		muxer.Fail("muxer has no backend");
		return muxer;
	}
	muxer.m_backend->BindAttachments(muxer, file);
	return muxer;
}

Muxer& StormByte::Multimedia::Pipeline::operator>>(Demuxer& demuxer, Muxer& muxer) noexcept {
	if (!muxer.m_plan)
		muxer.m_plan = demuxer.m_plan;
	if (muxer.Failed())
		return muxer;
	if (!demuxer.Plan()) {
		muxer.Fail("demuxer has no plan");
		return muxer;
	}
	if (!muxer.m_backend) {
		muxer.Fail("muxer has no backend");
		return muxer;
	}
	muxer.m_origin = &demuxer;
	muxer.m_backend->BindAttachments(muxer, demuxer.OriginFile());
	return muxer;
}
