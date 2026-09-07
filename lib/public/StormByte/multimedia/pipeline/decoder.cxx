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

#include <StormByte/multimedia/backend/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/backend/ffmpeg/AVSubtitle.hxx>
#include <StormByte/multimedia/pipeline/decoder.hxx>
#include <StormByte/multimedia/pipeline/decoder_impl.hxx>
#include <StormByte/multimedia/pipeline/frame_impl.hxx>
#include <StormByte/multimedia/property/hdr10.hxx>
#include <StormByte/multimedia/property/point.hxx>

#include <cstdint>
#include <string>

extern "C" {
	#include <libavutil/avutil.h>
	#include <libavutil/frame.h>
	#include <libavutil/mastering_display_metadata.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/rational.h>
}

using namespace StormByte::Multimedia::Pipeline;
namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;

namespace {
	constexpr int ChromaDenominator = 50000;

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

	std::int64_t NsToTicks(const std::optional<StormByte::Multimedia::Property::Duration>& value, AVRational timeBase) noexcept {
		if (!value.has_value() || timeBase.num <= 0 || timeBase.den <= 0)
			return AV_NOPTS_VALUE;
		return av_rescale_q(value->Nanoseconds().count(), AVRational{1, 1000000000}, timeBase);
	}

	StormByte::Multimedia::Property::Point FromRationalPair(const AVRational& x, const AVRational& y) noexcept {
		return StormByte::Multimedia::Property::Point::Normalized(x.num, x.den, y.num, y.den, ChromaDenominator);
	}

	SideDataKind MapKind(AVFrameSideDataType type) noexcept {
		switch (type) {
			case AV_FRAME_DATA_MASTERING_DISPLAY_METADATA:	return SideDataKind::MasteringDisplay;
			case AV_FRAME_DATA_CONTENT_LIGHT_LEVEL:			return SideDataKind::ContentLight;
			case AV_FRAME_DATA_DYNAMIC_HDR_PLUS:			return SideDataKind::HdrPlus;
			case AV_FRAME_DATA_DYNAMIC_HDR_VIVID:			return SideDataKind::HdrVivid;
			case AV_FRAME_DATA_A53_CC:						return SideDataKind::A53CC;
			case AV_FRAME_DATA_STEREO3D:					return SideDataKind::Stereo3D;
			case AV_FRAME_DATA_DISPLAYMATRIX:				return SideDataKind::DisplayMatrix;
			case AV_FRAME_DATA_ICC_PROFILE:					return SideDataKind::IccProfile;
			case AV_FRAME_DATA_S12M_TIMECODE:				return SideDataKind::S12MTimecode;
			case AV_FRAME_DATA_SPHERICAL:					return SideDataKind::Spherical;
			case AV_FRAME_DATA_SEI_UNREGISTERED:			return SideDataKind::SeiUnregistered;
			case AV_FRAME_DATA_FILM_GRAIN_PARAMS:			return SideDataKind::FilmGrain;
			case AV_FRAME_DATA_DOVI_RPU_BUFFER:				return SideDataKind::DolbyVisionRpu;
			case AV_FRAME_DATA_DOVI_METADATA:				return SideDataKind::DolbyVision;
			case AV_FRAME_DATA_AMBIENT_VIEWING_ENVIRONMENT:	return SideDataKind::AmbientViewing;
			default:										return SideDataKind::Other;
		}
	}

	std::vector<SideData> MapAttachments(const ::AVFrame* av) noexcept {
		std::vector<SideData> out;
		if (!av)
			return out;
		for (int i = 0; i < av->nb_side_data; ++i) {
			const AVFrameSideData* sd = av->side_data[i];
			if (!sd || !sd->data || sd->size <= 0)
				continue;
			StormByte::Buffer::DataType bytes(
				reinterpret_cast<const std::byte*>(sd->data),
				reinterpret_cast<const std::byte*>(sd->data) + sd->size);
			const auto kind = MapKind(sd->type);
			if (kind == SideDataKind::Other) {
				const char* name = av_frame_side_data_name(sd->type);
				out.emplace_back(name ? std::string(name) : std::string("unknown"),
					StormByte::Buffer::FIFO{std::move(bytes)});
			}
			else
				out.emplace_back(kind, StormByte::Buffer::FIFO{std::move(bytes)});
		}
		return out;
	}

	std::optional<StormByte::Multimedia::Property::HDR10> MapFrameHDR10(
		const ::AVFrame* av, bool heuristics, const StormByte::Multimedia::Property::Video& video) noexcept {
		const AVFrameSideData* mdmSd = av ? av_frame_get_side_data(av, AV_FRAME_DATA_MASTERING_DISPLAY_METADATA) : nullptr;
		const AVFrameSideData* cllSd = av ? av_frame_get_side_data(av, AV_FRAME_DATA_CONTENT_LIGHT_LEVEL) : nullptr;
		const AVFrameSideData* plusSd = av ? av_frame_get_side_data(av, AV_FRAME_DATA_DYNAMIC_HDR_PLUS) : nullptr;
		const auto* mdm = (mdmSd && mdmSd->size >= sizeof(AVMasteringDisplayMetadata))
			? reinterpret_cast<const AVMasteringDisplayMetadata*>(mdmSd->data) : nullptr;
		const auto* cll = (cllSd && cllSd->size >= sizeof(AVContentLightMetadata))
			? reinterpret_cast<const AVContentLightMetadata*>(cllSd->data) : nullptr;
		const bool plus = plusSd != nullptr;

		std::optional<StormByte::Multimedia::Property::Point> light;
		if (cll && (cll->MaxCLL || cll->MaxFALL))
			light = StormByte::Multimedia::Property::Point{static_cast<int>(cll->MaxCLL), static_cast<int>(cll->MaxFALL)};

		if (mdm && mdm->has_primaries && mdm->has_luminance) {
			StormByte::Multimedia::Property::HDR10 out{
				FromRationalPair(mdm->display_primaries[0][0], mdm->display_primaries[0][1]),
				FromRationalPair(mdm->display_primaries[1][0], mdm->display_primaries[1][1]),
				FromRationalPair(mdm->display_primaries[2][0], mdm->display_primaries[2][1]),
				FromRationalPair(mdm->white_point[0], mdm->white_point[1]),
				FromRationalPair(mdm->min_luminance, mdm->max_luminance),
				light,
				StormByte::Multimedia::Property::HDR10::Source::Metadata
			};
			out.HDR10Plus(plus);
			return out;
		}

		if (heuristics && video.Color().IsHDR10()) {
			StormByte::Multimedia::Property::HDR10 out = StormByte::Multimedia::Property::HDR10::DEFAULT;
			if (light)
				out = StormByte::Multimedia::Property::HDR10{
					out.Red(), out.Green(), out.Blue(), out.White(), out.Luminance(),
					light, StormByte::Multimedia::Property::HDR10::Source::Heuristics
				};
			out.HDR10Plus(plus);
			return out;
		}
		if (video.HDR10()) {
			auto kept = *video.HDR10();
			if (plus)
				kept.HDR10Plus(true);
			return kept;
		}
		return std::nullopt;
	}
}

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

const std::optional<std::string>& Decoder::Implementation() const noexcept {
	return m_implementation;
}

void Decoder::Implementation(std::string name) noexcept {
	if (name.empty())
		m_implementation.reset();
	else
		m_implementation = std::move(name);
}

const StormByte::Multimedia::Features& Decoder::Require() const noexcept {
	return m_require;
}

void Decoder::Require(StormByte::Multimedia::Features features) noexcept {
	m_require = features;
}

const StormByte::Multimedia::Features& Decoder::Capabilities() const noexcept {
	return m_capabilities;
}

Filter::FramePipe& Decoder::Pipe() noexcept {
	return m_pipe;
}

const Filter::FramePipe& Decoder::Pipe() const noexcept {
	return m_pipe;
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
	m_capabilities = StormByte::Multimedia::Features{};
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
	if (!m_impl->m_decoder.IsSubtitle())
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

	const auto tb = decoder.m_impl->m_timeBase;
	const std::int64_t duration = packet.Duration()
		? av_rescale_q(packet.Duration()->Nanoseconds().count(), AVRational{1, 1000000000}, tb)
		: 0;
	raw.Timestamps(NsToTicks(packet.Pts(), tb), NsToTicks(packet.Dts(), tb), duration);

	if (decoder.m_impl->m_decoder.IsSubtitle()) {
		FFmpeg::AVSubtitle sub;
		const auto result = decoder.m_impl->m_decoder.DecodeSubtitle(raw, sub);
		if (result == FFmpeg::OperationResult::Error)
			decoder.Fail("failed to decode subtitle");
		else if (result == FFmpeg::OperationResult::Success)
			decoder.m_impl->m_pendingSub = std::move(sub);
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

	auto runPipe = [&]() {
		auto filtered = decoder.m_pipe.Push(std::move(frame));
		if (decoder.m_pipe.Failed()) {
			decoder.Fail(decoder.m_pipe.Error().value_or("frame filter failed"));
			frame = Frame{};
			return;
		}
		if (!filtered.has_value()) {
			frame = Frame{};
			return;
		}
		frame = std::move(*filtered);
	};

	if (decoder.m_impl->m_decoder.IsSubtitle()) {
		if (!decoder.m_impl->m_pendingSub.has_value())
			return decoder;

		auto sub = std::move(*decoder.m_impl->m_pendingSub);
		decoder.m_impl->m_pendingSub.reset();

		const auto text = sub.Text();
		StormByte::Buffer::DataType bytes(
			reinterpret_cast<const std::byte*>(text.data()),
			reinterpret_cast<const std::byte*>(text.data()) + text.size());

		auto pts = TicksToPts(sub.Pts(), AVRational{1, AV_TIME_BASE});
		std::optional<StormByte::Multimedia::Property::Duration> duration;
		if (sub.DisplayDurationMs() > 0)
			duration = StormByte::Multimedia::Property::Duration{
				std::chrono::milliseconds{sub.DisplayDurationMs()}};

		frame = Frame(
			decoder.m_index,
			StormByte::Buffer::FIFO{std::move(bytes)},
			std::move(pts),
			std::move(duration),
			std::nullopt,
			{}
		);
		runPipe();
		return decoder;
	}

	auto holder = std::make_unique<Frame::Impl>();
	const auto result = decoder.m_impl->m_decoder.ReceiveFrame(holder->m_backend);
	if (result == FFmpeg::OperationResult::TryAgain || result == FFmpeg::OperationResult::EndOfFile)
		return decoder;
	if (result != FFmpeg::OperationResult::Success) {
		decoder.Fail("failed to receive frame");
		return decoder;
	}

	const auto tb = decoder.m_impl->m_timeBase;
	auto video = decoder.m_impl->m_video;
	auto attachments = MapAttachments(holder->m_backend.Get());
	if (video) {
		auto hdr = MapFrameHDR10(holder->m_backend.Get(),
			decoder.m_flags.Has(DecoderFlag::HeuristicsHDR10), *video);
		video = StormByte::Multimedia::Property::Video(video->Color(), video->Resolution(), std::move(hdr));
	}

	frame = Frame(
		decoder.m_index,
		StormByte::Buffer::FIFO{},
		TicksToPts(holder->m_backend.Pts(), tb),
		TicksToDuration(holder->m_backend.DurationTicks(), tb),
		std::move(video),
		std::move(attachments),
		decoder.m_impl->m_audio
	);
	frame.Bind(std::move(holder));
	runPipe();
	return decoder;
}
