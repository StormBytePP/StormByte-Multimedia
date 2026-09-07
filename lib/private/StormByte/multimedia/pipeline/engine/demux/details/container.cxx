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
#include <StormByte/multimedia/pipeline/engine/demux/details/container.hxx>

#include <cstdint>

extern "C" {
	#include <libavcodec/packet.h>
	#include <libavutil/avutil.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/rational.h>
}

namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;
namespace Details = StormByte::Multimedia::Pipeline::Engine::Demux::Details;

namespace {
	std::optional<StormByte::Multimedia::Property::Duration> TicksToPts(std::int64_t ticks, AVRational timeBase) noexcept {
		if (ticks == AV_NOPTS_VALUE || ticks < 0 || timeBase.num <= 0 || timeBase.den <= 0)
			return std::nullopt;
		const std::int64_t ns = av_rescale_q(ticks, timeBase, AVRational{1, 1000000000});
		if (ns < 0)
			return std::nullopt;
		return StormByte::Multimedia::Property::Duration{std::chrono::nanoseconds{ns}};
	}

	std::optional<StormByte::Multimedia::Property::Duration> TicksToDuration(std::int64_t ticks, AVRational timeBase) noexcept {
		if (ticks == AV_NOPTS_VALUE || ticks <= 0 || timeBase.num <= 0 || timeBase.den <= 0)
			return std::nullopt;
		const std::int64_t ns = av_rescale_q(ticks, timeBase, AVRational{1, 1000000000});
		if (ns <= 0)
			return std::nullopt;
		return StormByte::Multimedia::Property::Duration{std::chrono::nanoseconds{ns}};
	}
}

Details::Container::Container() noexcept = default;

bool Details::Container::IsOpen() const noexcept {
	return m_ctx.has_value();
}

bool Details::Container::Open(class Demux& owner, const File&) noexcept {
	owner.Fail("use file >> demux");
	return false;
}

bool Details::Container::Adopt(class Demux& owner, FFmpeg::AVFormatContext ctx) noexcept {
	m_ctx = std::move(ctx);
	m_timeBase.clear();
	if (!m_ctx) {
		owner.Fail("demuxer is not open");
		return false;
	}
	for (const auto& stream : m_ctx->Streams())
		m_timeBase[stream.Index()] = stream.TimeBase();
	return true;
}

bool Details::Container::Read(class Demux& owner, Packet& packet) noexcept {
	if (!m_ctx) {
		owner.Fail("demuxer is not open");
		return false;
	}
	for (;;) {
		const auto result = m_ctx->ReadPacket(m_scratch);
		if (result == FFmpeg::OperationResult::EndOfFile) {
			owner.ReachedEof();
			return false;
		}
		if (result == FFmpeg::OperationResult::TryAgain)
			continue;
		if (result != FFmpeg::OperationResult::Success) {
			owner.Fail("failed to read packet");
			return false;
		}

		const int index = m_scratch.StreamIndex();
		AVRational tb{0, 1};
		if (const auto found = m_timeBase.find(index); found != m_timeBase.end())
			tb = found->second;

		StormByte::Buffer::DataType bytes;
		const auto* data = m_scratch.Data();
		const int size = m_scratch.Size();
		if (data && size > 0) {
			const auto* raw = reinterpret_cast<const std::byte*>(data);
			bytes.assign(raw, raw + size);
		}

		packet = Packet{
			index,
			StormByte::Buffer::FIFO{std::move(bytes)},
			TicksToPts(m_scratch.Pts(), tb),
			TicksToPts(m_scratch.Dts(), tb),
			TicksToDuration(m_scratch.Duration(), tb),
			(m_scratch.Flags() & AV_PKT_FLAG_KEY) != 0
		};
		m_scratch.Unref();
		return true;
	}
}

void* Details::Container::Context() noexcept {
	return m_ctx ? m_ctx->Get() : nullptr;
}

StormByte::Multimedia::Backend::FFmpeg::AVFormatContext* Details::Container::Format() noexcept {
	return m_ctx ? &*m_ctx : nullptr;
}

const std::unordered_map<int, AVRational>& Details::Container::TimeBases() const noexcept {
	return m_timeBase;
}
