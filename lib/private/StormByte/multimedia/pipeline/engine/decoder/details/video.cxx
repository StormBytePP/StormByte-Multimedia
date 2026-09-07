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
#include <StormByte/multimedia/pipeline/engine/decoder/details/video.hxx>
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/property/hdr10.hxx>
#include <StormByte/multimedia/property/point.hxx>

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

extern "C" {
	#include <libavutil/avutil.h>
	#include <libavutil/frame.h>
	#include <libavutil/mastering_display_metadata.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/rational.h>
}

namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;
namespace Details = StormByte::Multimedia::Pipeline::Engine::Decoder::Details;

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

	StormByte::Multimedia::Pipeline::SideDataKind MapKind(AVFrameSideDataType type) noexcept {
		switch (type) {
			case AV_FRAME_DATA_MASTERING_DISPLAY_METADATA:	return StormByte::Multimedia::Pipeline::SideDataKind::MasteringDisplay;
			case AV_FRAME_DATA_CONTENT_LIGHT_LEVEL:			return StormByte::Multimedia::Pipeline::SideDataKind::ContentLight;
			case AV_FRAME_DATA_DYNAMIC_HDR_PLUS:			return StormByte::Multimedia::Pipeline::SideDataKind::HdrPlus;
			case AV_FRAME_DATA_DYNAMIC_HDR_VIVID:			return StormByte::Multimedia::Pipeline::SideDataKind::HdrVivid;
			case AV_FRAME_DATA_A53_CC:						return StormByte::Multimedia::Pipeline::SideDataKind::A53CC;
			case AV_FRAME_DATA_STEREO3D:					return StormByte::Multimedia::Pipeline::SideDataKind::Stereo3D;
			case AV_FRAME_DATA_DISPLAYMATRIX:				return StormByte::Multimedia::Pipeline::SideDataKind::DisplayMatrix;
			case AV_FRAME_DATA_ICC_PROFILE:					return StormByte::Multimedia::Pipeline::SideDataKind::IccProfile;
			case AV_FRAME_DATA_S12M_TIMECODE:				return StormByte::Multimedia::Pipeline::SideDataKind::S12MTimecode;
			case AV_FRAME_DATA_SPHERICAL:					return StormByte::Multimedia::Pipeline::SideDataKind::Spherical;
			case AV_FRAME_DATA_SEI_UNREGISTERED:			return StormByte::Multimedia::Pipeline::SideDataKind::SeiUnregistered;
			case AV_FRAME_DATA_FILM_GRAIN_PARAMS:			return StormByte::Multimedia::Pipeline::SideDataKind::FilmGrain;
			case AV_FRAME_DATA_DOVI_RPU_BUFFER:				return StormByte::Multimedia::Pipeline::SideDataKind::DolbyVisionRpu;
			case AV_FRAME_DATA_DOVI_METADATA:				return StormByte::Multimedia::Pipeline::SideDataKind::DolbyVision;
			case AV_FRAME_DATA_AMBIENT_VIEWING_ENVIRONMENT:	return StormByte::Multimedia::Pipeline::SideDataKind::AmbientViewing;
			default:										return StormByte::Multimedia::Pipeline::SideDataKind::Other;
		}
	}

	std::vector<StormByte::Multimedia::Pipeline::SideData> MapAttachments(const ::AVFrame* av) noexcept {
		std::vector<StormByte::Multimedia::Pipeline::SideData> out;
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
			if (kind == StormByte::Multimedia::Pipeline::SideDataKind::Other) {
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

	void StampTags(class StormByte::Multimedia::Pipeline::Decoder& decoder,
		class StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
		if (decoder.Language())
			frame.Language(*decoder.Language());
		if (decoder.Title())
			frame.Title(*decoder.Title());
	}
}

Details::Video::Video(FFmpeg::AVDecoder decoder, AVRational timeBase,
	std::optional<StormByte::Multimedia::Property::Video> video) noexcept
: m_decoder(std::move(decoder)), m_video(std::move(video)), m_timeBase(timeBase) {}

bool Details::Video::IsOpen() const noexcept {
	return true;
}

bool Details::Video::Send(class Decoder& owner, Packet& packet) noexcept {
	FFmpeg::AVPacket raw;
	StormByte::Buffer::DataType bytes;
	const auto n = packet.Payload().AvailableBytes();
	const std::uint8_t* data = nullptr;
	if (n > 0) {
		if (!packet.Payload().Extract(n, bytes) || bytes.size() != n) {
			owner.Fail("failed to extract packet payload");
			return false;
		}
		data = reinterpret_cast<const std::uint8_t*>(bytes.data());
	}
	if (!raw.Load(data, static_cast<int>(n), owner.Index(), packet.KeyFrame())) {
		owner.Fail("out of memory copying packet");
		return false;
	}

	const std::int64_t duration = packet.Duration()
		? av_rescale_q(packet.Duration()->Nanoseconds().count(), AVRational{1, 1000000000}, m_timeBase)
		: 0;
	raw.Timestamps(NsToTicks(packet.Pts(), m_timeBase), NsToTicks(packet.Dts(), m_timeBase), duration);

	auto result = m_decoder.SendPacket(raw);
	while (result == FFmpeg::OperationResult::TryAgain) {
		StormByte::Multimedia::Pipeline::Frame ignored;
		if (!Receive(owner, ignored))
			break;
		if (owner.Failed())
			return false;
		result = m_decoder.SendPacket(raw);
	}
	if (result == FFmpeg::OperationResult::Error) {
		owner.Fail("failed to send packet");
		return false;
	}
	return true;
}

bool Details::Video::Receive(class Decoder& owner, class Frame& frame) noexcept {
	auto holder = std::make_unique<StormByte::Multimedia::Pipeline::Engine::Frame::Engine>();
	const auto result = m_decoder.ReceiveFrame(holder->m_backend);
	if (result == FFmpeg::OperationResult::TryAgain || result == FFmpeg::OperationResult::EndOfFile)
		return false;
	if (result != FFmpeg::OperationResult::Success) {
		owner.Fail("failed to receive frame");
		return false;
	}

	auto video = m_video;
	auto attachments = MapAttachments(holder->m_backend.Get());
	if (video) {
		auto hdr = MapFrameHDR10(holder->m_backend.Get(),
			owner.Flags().Has(StormByte::Multimedia::Pipeline::DecoderFlag::HeuristicsHDR10), *video);
		video = StormByte::Multimedia::Property::Video(video->Color(), video->Resolution(), std::move(hdr));
	}

	frame = StormByte::Multimedia::Pipeline::Frame(
		owner.Index(),
		StormByte::Buffer::FIFO{},
		TicksToPts(holder->m_backend.Pts(), m_timeBase),
		TicksToDuration(holder->m_backend.DurationTicks(), m_timeBase),
		std::move(video),
		std::move(attachments),
		std::nullopt
	);
	StampTags(owner, frame);
	frame.Bind(std::move(holder));
	return true;
}

void Details::Video::Flush(class Decoder& owner) noexcept {
	if (owner.Failed() || m_flushed)
		return;
	for (;;) {
		const auto sent = m_decoder.SetEof();
		if (sent == FFmpeg::OperationResult::Success || sent == FFmpeg::OperationResult::EndOfFile)
			break;
		if (sent == FFmpeg::OperationResult::TryAgain)
			break;
		owner.Fail("failed to signal decoder EOF");
		return;
	}
	m_flushed = true;
}
