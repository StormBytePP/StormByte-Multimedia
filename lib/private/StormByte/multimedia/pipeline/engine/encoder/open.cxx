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

#include <StormByte/multimedia/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/open.hxx>
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>
#include <StormByte/multimedia/pipeline/side_data.hxx>
#include <StormByte/multimedia/property/channel_layout.hxx>
#include <StormByte/multimedia/property/color.hxx>
#include <StormByte/multimedia/type.hxx>
#include <tables/encoder/table.hxx>

#include <cstdint>
#include <cstring>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavcodec/packet.h>
	#include <libavutil/avutil.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/mastering_display_metadata.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/pixfmt.h>
	#include <libavutil/rational.h>
	#include <libavutil/samplefmt.h>
}

namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;
namespace Prop = StormByte::Multimedia::Property;
using StormByte::Multimedia::Pipeline::Encoder;
using StormByte::Multimedia::Pipeline::Frame;
using StormByte::Multimedia::Pipeline::Packet;
using StormByte::Multimedia::Pipeline::SideData;
using StormByte::Multimedia::Pipeline::SideDataKind;

namespace {
	constexpr AVRational NanoTimeBase{1, 1000000000};

	int ToAVPixelFormat(Prop::PixelFormat format) noexcept {
		switch (format) {
			case Prop::PixelFormat::YUV420P:	return AV_PIX_FMT_YUV420P;
			case Prop::PixelFormat::YUV422P:	return AV_PIX_FMT_YUV422P;
			case Prop::PixelFormat::YUV444P:	return AV_PIX_FMT_YUV444P;
			case Prop::PixelFormat::YUV420P10:	return AV_PIX_FMT_YUV420P10LE;
			case Prop::PixelFormat::YUV422P10:	return AV_PIX_FMT_YUV422P10LE;
			case Prop::PixelFormat::YUV444P10:	return AV_PIX_FMT_YUV444P10LE;
			case Prop::PixelFormat::YUV420P12:	return AV_PIX_FMT_YUV420P12LE;
			case Prop::PixelFormat::YUV422P12:	return AV_PIX_FMT_YUV422P12LE;
			case Prop::PixelFormat::YUV444P12:	return AV_PIX_FMT_YUV444P12LE;
			case Prop::PixelFormat::NV12:		return AV_PIX_FMT_NV12;
			case Prop::PixelFormat::NV21:		return AV_PIX_FMT_NV21;
			case Prop::PixelFormat::P010:		return AV_PIX_FMT_P010LE;
			case Prop::PixelFormat::RGB24:		return AV_PIX_FMT_RGB24;
			case Prop::PixelFormat::BGR24:		return AV_PIX_FMT_BGR24;
			case Prop::PixelFormat::RGBA:		return AV_PIX_FMT_RGBA;
			case Prop::PixelFormat::BGRA:		return AV_PIX_FMT_BGRA;
			case Prop::PixelFormat::GRAY8:		return AV_PIX_FMT_GRAY8;
			case Prop::PixelFormat::GRAY10:		return AV_PIX_FMT_GRAY10LE;
			case Prop::PixelFormat::GRAY16:		return AV_PIX_FMT_GRAY16LE;
			default:							return AV_PIX_FMT_NONE;
		}
	}

	int ToAVRange(Prop::Range range) noexcept {
		switch (range) {
			case Prop::Range::TV:	return AVCOL_RANGE_MPEG;
			case Prop::Range::Full:	return AVCOL_RANGE_JPEG;
			default:				return AVCOL_RANGE_UNSPECIFIED;
		}
	}

	int ToAVSpace(Prop::Space space) noexcept {
		switch (space) {
			case Prop::Space::RGB:					return AVCOL_SPC_RGB;
			case Prop::Space::BT709:				return AVCOL_SPC_BT709;
			case Prop::Space::FCC:					return AVCOL_SPC_FCC;
			case Prop::Space::BT470BG:				return AVCOL_SPC_BT470BG;
			case Prop::Space::SMPTE170M:			return AVCOL_SPC_SMPTE170M;
			case Prop::Space::SMPTE240M:			return AVCOL_SPC_SMPTE240M;
			case Prop::Space::YCgCo:				return AVCOL_SPC_YCGCO;
			case Prop::Space::BT2020NCL:			return AVCOL_SPC_BT2020_NCL;
			case Prop::Space::BT2020CL:				return AVCOL_SPC_BT2020_CL;
			case Prop::Space::SMPTE2085:			return AVCOL_SPC_SMPTE2085;
			case Prop::Space::ChromaDerivedNCL:		return AVCOL_SPC_CHROMA_DERIVED_NCL;
			case Prop::Space::ChromaDerivedCL:		return AVCOL_SPC_CHROMA_DERIVED_CL;
			case Prop::Space::ICtCp:				return AVCOL_SPC_ICTCP;
			default:								return AVCOL_SPC_UNSPECIFIED;
		}
	}

	int ToAVPrimaries(Prop::Primaries primaries) noexcept {
		switch (primaries) {
			case Prop::Primaries::BT709:		return AVCOL_PRI_BT709;
			case Prop::Primaries::BT470M:		return AVCOL_PRI_BT470M;
			case Prop::Primaries::BT470BG:		return AVCOL_PRI_BT470BG;
			case Prop::Primaries::SMPTE170M:	return AVCOL_PRI_SMPTE170M;
			case Prop::Primaries::SMPTE240M:	return AVCOL_PRI_SMPTE240M;
			case Prop::Primaries::Film:			return AVCOL_PRI_FILM;
			case Prop::Primaries::BT2020:		return AVCOL_PRI_BT2020;
			case Prop::Primaries::SMPTE428:		return AVCOL_PRI_SMPTE428;
			case Prop::Primaries::SMPTE431:		return AVCOL_PRI_SMPTE431;
			case Prop::Primaries::SMPTE432:		return AVCOL_PRI_SMPTE432;
			case Prop::Primaries::EBU3213:		return AVCOL_PRI_EBU3213;
			default:							return AVCOL_PRI_UNSPECIFIED;
		}
	}

	int ToAVTransfer(Prop::Transfer transfer) noexcept {
		switch (transfer) {
			case Prop::Transfer::BT709:			return AVCOL_TRC_BT709;
			case Prop::Transfer::Gamma22:		return AVCOL_TRC_GAMMA22;
			case Prop::Transfer::Gamma28:		return AVCOL_TRC_GAMMA28;
			case Prop::Transfer::SMPTE170M:		return AVCOL_TRC_SMPTE170M;
			case Prop::Transfer::SMPTE240M:		return AVCOL_TRC_SMPTE240M;
			case Prop::Transfer::Linear:			return AVCOL_TRC_LINEAR;
			case Prop::Transfer::Log:			return AVCOL_TRC_LOG;
			case Prop::Transfer::LogSqrt:		return AVCOL_TRC_LOG_SQRT;
			case Prop::Transfer::IEC61966_2_4:	return AVCOL_TRC_IEC61966_2_4;
			case Prop::Transfer::BT1361:		return AVCOL_TRC_BT1361_ECG;
			case Prop::Transfer::IEC61966_2_1:	return AVCOL_TRC_IEC61966_2_1;
			case Prop::Transfer::BT2020_10:		return AVCOL_TRC_BT2020_10;
			case Prop::Transfer::BT2020_12:		return AVCOL_TRC_BT2020_12;
			case Prop::Transfer::SMPTE2084:		return AVCOL_TRC_SMPTE2084;
			case Prop::Transfer::SMPTE428:		return AVCOL_TRC_SMPTE428;
			case Prop::Transfer::ARIB_B67:		return AVCOL_TRC_ARIB_STD_B67;
			default:							return AVCOL_TRC_UNSPECIFIED;
		}
	}

	std::optional<StormByte::Multimedia::Property::Duration> TicksToPts(std::int64_t ticks, AVRational timeBase) noexcept {
		if (ticks == AV_NOPTS_VALUE || ticks < 0 || timeBase.num <= 0 || timeBase.den <= 0)
			return std::nullopt;
		const std::int64_t ns = av_rescale_q(ticks, timeBase, NanoTimeBase);
		if (ns < 0)
			return std::nullopt;
		return StormByte::Multimedia::Property::Duration{std::chrono::nanoseconds{ns}};
	}

	std::optional<StormByte::Multimedia::Property::Duration> TicksToDuration(std::int64_t ticks, AVRational timeBase) noexcept {
		if (ticks == AV_NOPTS_VALUE || ticks <= 0 || timeBase.num <= 0 || timeBase.den <= 0)
			return std::nullopt;
		const std::int64_t ns = av_rescale_q(ticks, timeBase, NanoTimeBase);
		if (ns <= 0)
			return std::nullopt;
		return StormByte::Multimedia::Property::Duration{std::chrono::nanoseconds{ns}};
	}

	AVRational VideoTimeBaseFromFrame(const Frame& frame) noexcept {
		if (frame.Video() && frame.Video()->FrameRate() && frame.Video()->FrameRate()->Valid()) {
			const auto& fps = *frame.Video()->FrameRate();
			return AVRational{fps.Den(), fps.Num()};
		}
		return AVRational{1, 24};
	}

	AVRational ChooseTimeBase(const Frame& frame, StormByte::Multimedia::Type type) noexcept {
		if (type == StormByte::Multimedia::Type::Subtitle)
			return AVRational{1, AV_TIME_BASE};
		if (frame.Audio()) {
			const int rate = static_cast<int>(frame.Audio()->SampleRate());
			if (rate > 0)
				return AVRational{1, rate};
		}
		return VideoTimeBaseFromFrame(frame);
	}

	StormByte::Multimedia::Features FrameNeed(
		const StormByte::Multimedia::Features& extra,
		const Frame& frame) noexcept {
		StormByte::Multimedia::Features need = extra;
		if (frame.Video() && frame.Video()->HDR10())
			need.Add(StormByte::Multimedia::Feature::HDR10);
		return need;
	}

	const StormByte::Multimedia::Tables::Encoder::EncoderDef* ScanRows(
		std::span<const StormByte::Multimedia::Tables::Encoder::EncoderDef> rows,
		std::string_view codec, std::string_view pin, bool matchNeed,
		const StormByte::Multimedia::Features& need) noexcept {
		const StormByte::Multimedia::Tables::Encoder::EncoderDef* best = nullptr;
		for (const auto& row : rows) {
			if (codec != row.codec)
				continue;
			if (!pin.empty() && pin != row.name)
				continue;
			if (matchNeed && !row.features.Has(need))
				continue;
			if (matchNeed && avcodec_find_encoder_by_name(row.name) == nullptr)
				continue;
			if (!matchNeed)
				return &row;
			if (!best || row.preference < best->preference)
				best = &row;
		}
		return best;
	}

	const StormByte::Multimedia::Tables::Encoder::EncoderDef* FindRow(
		std::string_view codec, std::string_view pin) noexcept {
		if (const auto* row = ScanRows(StormByte::Multimedia::Tables::Encoder::Video(), codec, pin, false, {}))
			return row;
		if (const auto* row = ScanRows(StormByte::Multimedia::Tables::Encoder::Audio(), codec, pin, false, {}))
			return row;
		return ScanRows(StormByte::Multimedia::Tables::Encoder::Subtitle(), codec, pin, false, {});
	}

	const StormByte::Multimedia::Tables::Encoder::EncoderDef* PickEncoder(
		std::string_view codec, std::string_view pin, const StormByte::Multimedia::Features& need) noexcept {
		if (const auto* row = ScanRows(StormByte::Multimedia::Tables::Encoder::Video(), codec, pin, true, need))
			return row;
		if (const auto* row = ScanRows(StormByte::Multimedia::Tables::Encoder::Audio(), codec, pin, true, need))
			return row;
		return ScanRows(StormByte::Multimedia::Tables::Encoder::Subtitle(), codec, pin, true, need);
	}

	std::vector<std::pair<std::string, std::string>> SplitBlob(std::string_view blob) noexcept {
		std::vector<std::pair<std::string, std::string>> out;
		std::string_view rest = blob;
		while (!rest.empty()) {
			const auto cut = rest.find(':');
			const auto piece = rest.substr(0, cut);
			const auto eq = piece.find('=');
			if (eq != std::string_view::npos && eq > 0)
				out.emplace_back(std::string(piece.substr(0, eq)), std::string(piece.substr(eq + 1)));
			if (cut == std::string_view::npos)
				break;
			rest = rest.substr(cut + 1);
		}
		return out;
	}

	std::string JoinBlob(const std::map<std::string, std::string>& values) noexcept {
		std::string out;
		for (const auto& [key, value] : values) {
			if (key.empty())
				continue;
			if (!out.empty())
				out += ':';
			out += key;
			out += '=';
			out += value;
		}
		return out;
	}

	bool HasKey(const char* key) noexcept {
		return key && key[0] != '\0';
	}

	int PickAudioSampleFormat(const ::AVCodec* codec, int frameFormat) noexcept {
		if (!codec)
			return frameFormat;
		const void* cfg = nullptr;
		int count = 0;
		if (avcodec_get_supported_config(nullptr, codec, AV_CODEC_CONFIG_SAMPLE_FORMAT, 0, &cfg, &count) < 0
			|| !cfg || count <= 0)
			return frameFormat;
		const auto* fmts = static_cast<const AVSampleFormat*>(cfg);
		for (int i = 0; i < count; ++i) {
			if (fmts[i] == frameFormat)
				return frameFormat;
		}
		return static_cast<int>(fmts[0]);
	}

	bool LayoutSupported(const ::AVCodec* codec, const AVChannelLayout& layout) noexcept {
		if (!codec)
			return false;
		const void* cfg = nullptr;
		int count = 0;
		if (avcodec_get_supported_config(nullptr, codec, AV_CODEC_CONFIG_CHANNEL_LAYOUT, 0, &cfg, &count) < 0
			|| !cfg || count <= 0)
			return true;
		const auto* layouts = static_cast<const AVChannelLayout*>(cfg);
		for (int i = 0; i < count; ++i) {
			if (av_channel_layout_compare(&layouts[i], &layout) == 0)
				return true;
		}
		return false;
	}

	bool PickAudioLayout(const ::AVCodec* codec, const AVChannelLayout& src, AVChannelLayout& out) noexcept {
		av_channel_layout_uninit(&out);
		if (LayoutSupported(codec, src))
			return av_channel_layout_copy(&out, &src) >= 0;
		if (src.nb_channels <= 6)
			return false;
		if (av_channel_layout_from_mask(&out, AV_CH_LAYOUT_5POINT1) >= 0 && LayoutSupported(codec, out))
			return true;
		av_channel_layout_uninit(&out);
		if (av_channel_layout_from_mask(&out, AV_CH_LAYOUT_5POINT1_BACK) >= 0 && LayoutSupported(codec, out))
			return true;
		av_channel_layout_uninit(&out);
		return false;
	}

	std::int64_t EstimateCeiling(const Frame& frame) noexcept {
		int height = 0;
		if (frame.Video())
			height = static_cast<int>(frame.Video()->Resolution().Height());
		if (height <= 576)
			return 4000000;
		if (height <= 1080)
			return 12000000;
		if (height <= 1440)
			return 20000000;
		return 35000000;
	}

	std::int64_t BufSizeBits(const class Encoder& encoder, const Frame& frame) noexcept {
		if (encoder.MaxBitRate())
			return *encoder.MaxBitRate() * 2;
		if (encoder.BitRate())
			return *encoder.BitRate() * 2;
		return EstimateCeiling(frame) * 2;
	}

	void AddHdr10SideData(::AVCodecParameters* par, const StormByte::Multimedia::Property::HDR10& hdr10) noexcept {
		if (!par)
			return;

		const bool hasMastering = hdr10.Red().X() != 0 || hdr10.Red().Y() != 0
			|| hdr10.Green().X() != 0 || hdr10.Green().Y() != 0
			|| hdr10.Blue().X() != 0 || hdr10.Blue().Y() != 0
			|| hdr10.White().X() != 0 || hdr10.White().Y() != 0
			|| hdr10.Luminance().X() != 0 || hdr10.Luminance().Y() != 0;
		const bool hasLight = hdr10.LightLevel().has_value()
			&& (hdr10.LightLevel()->X() != 0 || hdr10.LightLevel()->Y() != 0);

		if (hasMastering) {
			AVMasteringDisplayMetadata mdm{};
			mdm.display_primaries[0][0] = av_make_q(static_cast<int>(hdr10.Red().X()), 50000);
			mdm.display_primaries[0][1] = av_make_q(static_cast<int>(hdr10.Red().Y()), 50000);
			mdm.display_primaries[1][0] = av_make_q(static_cast<int>(hdr10.Green().X()), 50000);
			mdm.display_primaries[1][1] = av_make_q(static_cast<int>(hdr10.Green().Y()), 50000);
			mdm.display_primaries[2][0] = av_make_q(static_cast<int>(hdr10.Blue().X()), 50000);
			mdm.display_primaries[2][1] = av_make_q(static_cast<int>(hdr10.Blue().Y()), 50000);
			mdm.white_point[0] = av_make_q(static_cast<int>(hdr10.White().X()), 50000);
			mdm.white_point[1] = av_make_q(static_cast<int>(hdr10.White().Y()), 50000);
			mdm.min_luminance = av_make_q(static_cast<int>(hdr10.Luminance().X()), 10000);
			mdm.max_luminance = av_make_q(static_cast<int>(hdr10.Luminance().Y()), 10000);
			mdm.has_primaries = 1;
			mdm.has_luminance = 1;
			if (AVPacketSideData* sd = av_packet_side_data_new(&par->coded_side_data, &par->nb_coded_side_data,
					AV_PKT_DATA_MASTERING_DISPLAY_METADATA, sizeof(mdm), 0))
				std::memcpy(sd->data, &mdm, sizeof(mdm));
		}

		if (hasLight) {
			AVContentLightMetadata cll{};
			cll.MaxCLL = static_cast<unsigned>(hdr10.LightLevel()->X());
			cll.MaxFALL = static_cast<unsigned>(hdr10.LightLevel()->Y());
			if (AVPacketSideData* sd = av_packet_side_data_new(&par->coded_side_data, &par->nb_coded_side_data,
					AV_PKT_DATA_CONTENT_LIGHT_LEVEL, sizeof(cll), 0))
				std::memcpy(sd->data, &cll, sizeof(cll));
		}
	}

	FFmpeg::AVCodecParameters FillParams(const Frame& frame, const ::AVCodec* codec,
		const std::optional<std::int64_t>& bitRate, std::optional<int> sampleFormat) noexcept {
		FFmpeg::AVCodecParameters params(nullptr);
		if (codec) {
			params.CodecId(static_cast<int>(codec->id));
			params.CodecType(static_cast<int>(codec->type));
		}
		if (bitRate)
			params.BitRate(*bitRate);
		if (frame.Video()) {
			const auto& video = *frame.Video();
			params.Width(static_cast<int>(video.Resolution().Width()));
			params.Height(static_cast<int>(video.Resolution().Height()));
			params.Format(ToAVPixelFormat(video.Color().PixelFormat()));
			params.ColorRange(ToAVRange(video.Color().Range()));
			params.ColorSpace(ToAVSpace(video.Color().Space()));
			params.ColorPrimaries(ToAVPrimaries(video.Color().Primaries()));
			params.ColorTransfer(ToAVTransfer(video.Color().Transfer()));
			if (video.HDR10())
				AddHdr10SideData(params.Get(), *video.HDR10());
		}
		if (frame.Audio()) {
			const auto& audio = *frame.Audio();
			params.SampleRate(static_cast<int>(audio.SampleRate()));
			if (!bitRate)
				params.BitRate(static_cast<std::int64_t>(audio.BitRate()));
			int channels = static_cast<int>(audio.Channels());
			if (channels <= 0)
				channels = static_cast<int>(Prop::ChannelCount(audio.Layout()));
			params.DefaultChannelLayout(channels);
			if (sampleFormat)
				params.Format(*sampleFormat);
		}
		return params;
	}
}

std::int64_t StormByte::Multimedia::Pipeline::Engine::Encoder::Open::NsToTicks(std::int64_t ns, AVRational timeBase) noexcept {
	if (ns < 0 || timeBase.num <= 0 || timeBase.den <= 0)
		return AV_NOPTS_VALUE;
	return av_rescale_q(ns, NanoTimeBase, timeBase);
}

class Packet StormByte::Multimedia::Pipeline::Engine::Encoder::Open::MakePacket(
	enum Type type, int index, const FFmpeg::AVPacket& raw,
	AVRational timeBase, bool keepPacketHdrPlus) noexcept {
	StormByte::Buffer::DataType bytes;
	const auto* data = raw.Data();
	const int size = raw.Size();
	if (data && size > 0) {
		const auto* rawBytes = reinterpret_cast<const std::byte*>(data);
		bytes.assign(rawBytes, rawBytes + size);
	}

	std::vector<SideData> attachments;
	if (const auto* pkt = raw.Get()) {
		for (int i = 0; i < pkt->side_data_elems; ++i) {
			const AVPacketSideData& sd = pkt->side_data[i];
			if (!sd.data || sd.size <= 0)
				continue;
			if (sd.type == AV_PKT_DATA_DYNAMIC_HDR10_PLUS && !keepPacketHdrPlus)
				continue;
			if (sd.type != AV_PKT_DATA_DYNAMIC_HDR10_PLUS
				&& sd.type != AV_PKT_DATA_MASTERING_DISPLAY_METADATA
				&& sd.type != AV_PKT_DATA_CONTENT_LIGHT_LEVEL)
				continue;
			StormByte::Buffer::DataType blob(
				reinterpret_cast<const std::byte*>(sd.data),
				reinterpret_cast<const std::byte*>(sd.data) + sd.size);
			switch (sd.type) {
				case AV_PKT_DATA_DYNAMIC_HDR10_PLUS:
					attachments.emplace_back(SideDataKind::HdrPlus,
						StormByte::Buffer::FIFO{std::move(blob)});
					break;
				case AV_PKT_DATA_MASTERING_DISPLAY_METADATA:
					attachments.emplace_back(SideDataKind::MasteringDisplay,
						StormByte::Buffer::FIFO{std::move(blob)});
					break;
				case AV_PKT_DATA_CONTENT_LIGHT_LEVEL:
					attachments.emplace_back(SideDataKind::ContentLight,
						StormByte::Buffer::FIFO{std::move(blob)});
					break;
				default:
					break;
			}
		}
	}

	return StormByte::Multimedia::Pipeline::Packet{
		type,
		index,
		StormByte::Buffer::FIFO{std::move(bytes)},
		TicksToPts(raw.Pts(), timeBase),
		TicksToPts(raw.Dts(), timeBase),
		TicksToDuration(raw.Duration(), timeBase),
		(raw.Flags() & AV_PKT_FLAG_KEY) != 0,
		std::move(attachments)
	};
}

std::optional<StormByte::Multimedia::Pipeline::Engine::Encoder::Open::Backend>
StormByte::Multimedia::Pipeline::Engine::Encoder::Open::Access::Open(class Encoder& owner, const class Frame& frame) noexcept {
	if (!owner.Destination().HasAccess(StormByte::Multimedia::Operation::Write)) {
		owner.Fail("codec is not writable");
		return std::nullopt;
	}
	const auto kind = owner.Destination().Type();
	if (kind == StormByte::Multimedia::Type::Video && !frame.Video()) {
		owner.Fail("encoder destination is video but frame is not");
		return std::nullopt;
	}
	if (kind == StormByte::Multimedia::Type::Audio && !frame.Audio()) {
		owner.Fail("encoder destination is audio but frame is not");
		return std::nullopt;
	}
	if (kind != StormByte::Multimedia::Type::Subtitle
		&& (!frame.m_engine || !frame.m_engine->m_backend.Get())) {
		owner.Fail("frame has no backend buffer");
		return std::nullopt;
	}

	const bool hdr10 = frame.Video() && frame.Video()->HDR10().has_value();
	const bool hdr10plus = hdr10 && frame.Video()->HDR10()->IsHDR10Plus();
	const auto need = FrameNeed(owner.Require(), frame);
	const std::string stormName{owner.Destination().Name()};
	const std::string_view pin = owner.Implementation()
		? std::string_view{*owner.Implementation()} : std::string_view{};

	if (!pin.empty()) {
		if (avcodec_find_encoder_by_name(std::string(pin).c_str()) == nullptr) {
			owner.Fail("encoder implementation is unavailable");
			return std::nullopt;
		}
		const auto* listed = FindRow(stormName, pin);
		if (!listed || !listed->features.Has(need)) {
			owner.Fail("encoder implementation lacks required features");
			return std::nullopt;
		}
	}

	const auto* row = PickEncoder(stormName, pin, need);
	const ::AVCodec* codec = nullptr;
	if (row)
		codec = avcodec_find_encoder_by_name(row->name);
	if (!codec) {
		owner.Fail("no encoder for destination codec");
		return std::nullopt;
	}

	if (owner.CRF() && (!row || !HasKey(row->crf_key))) {
		owner.Fail("encoder implementation does not support CRF");
		return std::nullopt;
	}
	if (owner.BitRate() && (!row || !HasKey(row->bitrate_key))) {
		owner.Fail("encoder implementation does not support BitRate");
		return std::nullopt;
	}
	if (owner.MaxBitRate() && (!row || !HasKey(row->maxrate_key))) {
		owner.Fail("encoder implementation does not support MaxBitRate");
		return std::nullopt;
	}
	if (owner.Preset() && (!row || !HasKey(row->preset_key))) {
		owner.Fail("encoder implementation does not support Preset");
		return std::nullopt;
	}
	if (owner.Tune() && (!row || !HasKey(row->style_key))) {
		owner.Fail("encoder implementation does not support Tune");
		return std::nullopt;
	}

	for (const auto& [key, value] : owner.FineTune()) {
		(void)value;
		if (key == "bufsize" || key == "vbv-bufsize") {
			owner.Fail("FineTune cannot set bufsize");
			return std::nullopt;
		}
	}

	std::map<std::string, std::string> blob;
	std::map<std::string, std::string> opts;
	if (row && hdr10)
		for (auto& pair : SplitBlob(row->signal_hdr10 ? row->signal_hdr10 : ""))
			blob.insert(std::move(pair));
	if (row && hdr10plus)
		for (auto& pair : SplitBlob(row->signal_hdr10plus ? row->signal_hdr10plus : ""))
			blob.insert(std::move(pair));

	if (owner.CRF() && row && HasKey(row->crf_key))
		opts.emplace(row->crf_key, std::to_string(*owner.CRF()));
	if (owner.BitRate() && row && HasKey(row->bitrate_key))
		opts.emplace(row->bitrate_key, std::to_string(*owner.BitRate()));
	else if (owner.CRF() && !owner.BitRate() && row && HasKey(row->bitrate_key)
		&& (std::string_view(row->name) == "libvpx" || std::string_view(row->name) == "libvpx-vp9"))
		opts.emplace(row->bitrate_key, "0");
	if (owner.MaxBitRate() && row && HasKey(row->maxrate_key))
		opts.emplace(row->maxrate_key, std::to_string(*owner.MaxBitRate()));
	if (row && HasKey(row->bufsize_key) && frame.Video() && (owner.BitRate() || owner.MaxBitRate()))
		opts.emplace(row->bufsize_key, std::to_string(BufSizeBits(owner, frame)));
	if (owner.Preset() && row && HasKey(row->preset_key))
		opts.emplace(row->preset_key, *owner.Preset());
	if (owner.Tune() && row && HasKey(row->style_key))
		opts.emplace(row->style_key, *owner.Tune());

	const auto& fine = owner.FineTune();
	const bool pack = row && HasKey(row->tune_key);
	if (pack) {
		if (!blob.contains("wpp") && !fine.contains("wpp"))
			blob.emplace("wpp", "1");
		if (!blob.contains("pools") && !blob.contains("numa-pools")
			&& !fine.contains("pools") && !fine.contains("numa-pools"))
			blob.emplace("pools", "*");
	}
	if (row && (std::string_view(row->name) == "libvpx" || std::string_view(row->name) == "libvpx-vp9")) {
		if (!opts.contains("row-mt") && !fine.contains("row-mt"))
			opts.emplace("row-mt", "1");
	}

	for (const auto& [key, value] : fine) {
		if (key.empty())
			continue;
		if (blob.contains(key) && blob[key] != value) {
			owner.Fail("FineTune conflicts with HDR signaling key '" + key + "'");
			return std::nullopt;
		}
		if (opts.contains(key) && opts[key] != value) {
			owner.Fail("FineTune conflicts with encoder setter key '" + key + "'");
			return std::nullopt;
		}
		if (pack)
			blob.emplace(key, value);
		else
			opts.emplace(key, value);
	}

	if (pack) {
		const auto packed = JoinBlob(blob);
		if (!packed.empty())
			opts.emplace(row->tune_key, packed);
	}
	else {
		for (const auto& [key, value] : blob)
			opts.emplace(key, value);
	}

	std::optional<int> sampleFormat;
	if (frame.Audio() && frame.m_engine && frame.m_engine->m_backend.Get())
		sampleFormat = PickAudioSampleFormat(codec, frame.m_engine->m_backend.Get()->format);

	AVChannelLayout want{};
	if (kind == StormByte::Multimedia::Type::Audio
		&& frame.m_engine && frame.m_engine->m_backend.Get()) {
		if (!PickAudioLayout(codec, frame.m_engine->m_backend.Get()->ch_layout, want)) {
			owner.Fail("encoder does not support this channel layout");
			return std::nullopt;
		}
	}

	auto params = FillParams(frame, codec, owner.BitRate(), sampleFormat);
	if (want.nb_channels > 0 && params.Get())
		av_channel_layout_copy(&params.Get()->ch_layout, &want);
	av_channel_layout_uninit(&want);

	if (kind == StormByte::Multimedia::Type::Video
		&& frame.m_engine && frame.m_engine->m_backend.Get()) {
		const auto* raw = frame.m_engine->m_backend.Get();
		if (raw->format != AV_PIX_FMT_NONE)
			params.Format(raw->format);
		if (raw->width > 0)
			params.Width(raw->width);
		if (raw->height > 0)
			params.Height(raw->height);
	}
	const auto timeBase = ChooseTimeBase(frame, kind);
	auto backend = FFmpeg::AVEncoder::Open(const_cast<::AVCodec*>(codec), params, owner.Index(), opts, timeBase);
	if (!backend.has_value()) {
		owner.Fail(backend.error() ? backend.error()->what() : "Failed to open encoder");
		return std::nullopt;
	}

	Backend out{std::move(backend.value()), timeBase, {}, {}};
	out.timeBase = out.encoder.TimeBase();
	if (out.timeBase.num <= 0 || out.timeBase.den <= 0)
		out.timeBase = timeBase;
	if (row) {
		out.implementation = row->name;
		out.capabilities = row->features;
	}
	return out;
}
