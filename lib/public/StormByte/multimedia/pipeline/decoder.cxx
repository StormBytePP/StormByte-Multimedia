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

#include <StormByte/multimedia/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/decoder_impl.hxx>
#include <StormByte/multimedia/property/hdr10.hxx>

using namespace StormByte::Multimedia::Pipeline;
namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;

Decoder::Decoder(int stream_index, DecoderFlags flags) noexcept
: m_index(stream_index), m_flags(flags), m_failed(false) {}

Decoder::Decoder(Decoder&&) noexcept = default;
Decoder::~Decoder() noexcept = default;
Decoder& Decoder::operator=(Decoder&&) noexcept = default;

Decoder::operator bool() const noexcept {
	return !m_failed && static_cast<bool>(m_impl);
}

int Decoder::Index() const noexcept {
	return m_index;
}

const DecoderFlags& Decoder::Flags() const noexcept {
	return m_flags;
}

void Decoder::Flags(DecoderFlags flags) noexcept {
	m_flags = flags;
}

bool Decoder::Failed() const noexcept {
	return m_failed;
}

const std::optional<std::string>& Decoder::Error() const noexcept {
	return m_error;
}

void Decoder::Fail(std::string reason) noexcept {
	m_failed = true;
	m_error = std::move(reason);
	m_impl.reset();
}

void Decoder::Bind(std::unique_ptr<Impl> impl) noexcept {
	m_impl = std::move(impl);
	m_failed = false;
	m_error.reset();
}

void Decoder::Flush() noexcept {
	if (m_failed || !m_impl)
		return;
	m_impl->m_decoder.SetEof();
}

Packet& StormByte::Multimedia::Pipeline::operator>>(Packet& packet, Decoder& decoder) noexcept {
	if (decoder.m_failed || !decoder.m_impl)
		return packet;
	if (packet.StreamIndex() != decoder.m_index)
		return packet;

	FFmpeg::AVPacket raw;
	StormByte::Buffer::DataType bytes;
	const auto n = packet.Payload().AvailableBytes();
	const std::uint8_t* data = nullptr;
	if (n > 0) {
		if (!packet.Payload().Read(n, bytes) || bytes.size() != n) {
			decoder.Fail("failed to read packet payload");
			return packet;
		}
		data = reinterpret_cast<const std::uint8_t*>(bytes.data());
	}
	if (!raw.Load(data, static_cast<int>(n), decoder.m_index, packet.KeyFrame())) {
		decoder.Fail("out of memory copying packet");
		return packet;
	}

	auto result = decoder.m_impl->m_decoder.SendPacket(raw);
	while (result == FFmpeg::OperationResult::TryAgain) {
		StormByte::Multimedia::Pipeline::Frame ignored;
		decoder >> ignored;
		if (decoder.m_failed)
			return packet;
		result = decoder.m_impl->m_decoder.SendPacket(raw);
	}
	if (result == FFmpeg::OperationResult::Error)
		decoder.Fail("failed to send packet");
	return packet;
}

Decoder& StormByte::Multimedia::Pipeline::operator>>(Decoder& decoder, Frame& frame) noexcept {
	if (decoder.m_failed || !decoder.m_impl)
		return decoder;

	const auto result = decoder.m_impl->m_decoder.ReceiveFrame(decoder.m_impl->m_scratch);
	if (result == FFmpeg::OperationResult::TryAgain || result == FFmpeg::OperationResult::EndOfFile)
		return decoder;
	if (result != FFmpeg::OperationResult::Success) {
		decoder.Fail("failed to receive frame");
		return decoder;
	}

	auto video = decoder.m_impl->m_video;
	if (video && decoder.m_flags.Has(DecoderFlag::HeuristicsHDR10)) {
		const auto& hdr = video->HDR10();
		if ((!hdr.has_value() || hdr->Origin() != StormByte::Multimedia::Property::HDR10::Source::Metadata)
			&& video->Color().IsHDR10())
			video = StormByte::Multimedia::Property::Video(
				video->Color(), video->Resolution(),
				StormByte::Multimedia::Property::HDR10::DEFAULT);
	}

	frame = Frame(
		decoder.m_index,
		StormByte::Buffer::FIFO{},
		std::nullopt,
		std::nullopt,
		std::move(video),
		{}
	);
	decoder.m_impl->m_scratch.Unref();
	return decoder;
}
