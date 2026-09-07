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
#include <StormByte/multimedia/backend/ffmpeg/AVSubtitle.hxx>
#include <StormByte/multimedia/ocr/bitmap.hxx>
#include <StormByte/multimedia/pipeline/encoder.hxx>
#include <StormByte/multimedia/pipeline/encoder_impl.hxx>
#include <StormByte/multimedia/pipeline/frame_impl.hxx>
#include <StormByte/multimedia/property/channel_layout.hxx>
#include <StormByte/multimedia/property/color.hxx>
#include <StormByte/multimedia/type.hxx>
#include <tables/encoder/table.hxx>

#include <cctype>
#include <cstdint>
#include <cstdio>
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
	#include <libavutil/mathematics.h>
	#include <libavutil/pixfmt.h>
	#include <libavutil/rational.h>
}

using namespace StormByte::Multimedia::Pipeline;
namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;
namespace Prop = StormByte::Multimedia::Property;

namespace {
	constexpr AVRational NanoTimeBase{1, 1000000000};
	constexpr std::size_t DialogueFieldsBeforeText = 9;
	constexpr std::size_t PackedAssFieldsBeforeText = 8;

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

	std::int64_t NsToTicks(std::int64_t ns, AVRational timeBase) noexcept {
		if (ns < 0 || timeBase.num <= 0 || timeBase.den <= 0)
			return AV_NOPTS_VALUE;
		return av_rescale_q(ns, NanoTimeBase, timeBase);
	}

	AVRational ChooseTimeBase(const Frame& frame, StormByte::Multimedia::Type type) noexcept {
		if (type == StormByte::Multimedia::Type::Subtitle)
			return AVRational{1, AV_TIME_BASE};
		if (frame.Audio()) {
			const int rate = static_cast<int>(frame.Audio()->SampleRate());
			if (rate > 0)
				return AVRational{1, rate};
		}
		return NanoTimeBase;
	}

	StormByte::Multimedia::Features FrameNeed(
		const StormByte::Multimedia::Features& extra,
		const Frame& frame) noexcept {
		StormByte::Multimedia::Features need = extra;
		if (!frame.Video() || !frame.Video()->HDR10())
			return need;
		need.Add(StormByte::Multimedia::Feature::HDR10);
		if (frame.Video()->HDR10()->IsHDR10Plus()) {
			need.Add(StormByte::Multimedia::Feature::HDR10Plus);
			need.Add(StormByte::Multimedia::Feature::SideData);
		}
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

	std::int64_t BufSizeBits(const Encoder& encoder, const Frame& frame) noexcept {
		if (encoder.MaxBitRate())
			return *encoder.MaxBitRate() * 2;
		if (encoder.BitRate())
			return *encoder.BitRate() * 2;
		return EstimateCeiling(frame) * 2;
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

	Packet MakePacket(int index, const FFmpeg::AVPacket& raw, AVRational timeBase) noexcept {
		StormByte::Buffer::DataType bytes;
		const auto* data = raw.Data();
		const int size = raw.Size();
		if (data && size > 0) {
			const auto* rawBytes = reinterpret_cast<const std::byte*>(data);
			bytes.assign(rawBytes, rawBytes + size);
		}
		return Packet{
			index,
			StormByte::Buffer::FIFO{std::move(bytes)},
			TicksToPts(raw.Pts(), timeBase),
			TicksToPts(raw.Dts(), timeBase),
			TicksToDuration(raw.Duration(), timeBase),
			(raw.Flags() & AV_PKT_FLAG_KEY) != 0
		};
	}

	std::string_view AfterCommas(std::string_view fields, std::size_t need) noexcept {
		std::size_t commas = 0;
		for (std::size_t i = 0; i < fields.size(); ++i) {
			if (fields[i] != ',')
				continue;
			++commas;
			if (commas == need)
				return fields.substr(i + 1);
		}
		return {};
	}

	bool LooksPackedAss(std::string_view in) noexcept {
		if (in.empty() || !std::isdigit(static_cast<unsigned char>(in.front())))
			return false;
		std::size_t commas = 0;
		for (const char c : in)
			if (c == ',')
				++commas;
		return commas >= PackedAssFieldsBeforeText;
	}

	std::string DialogueBody(std::string_view in) noexcept {
		while (!in.empty() && (in.front() == '\0' || in.front() == ' ' || in.front() == '\t'))
			in.remove_prefix(1);
		while (!in.empty() && (in.back() == '\0' || in.back() == '\n' || in.back() == '\r'))
			in.remove_suffix(1);

		const auto tag = in.find("Dialogue:");
		if (tag != std::string_view::npos) {
			auto fields = in.substr(tag + 9);
			while (!fields.empty() && (fields.front() == ' ' || fields.front() == '\t'))
				fields.remove_prefix(1);
			const auto body = AfterCommas(fields, DialogueFieldsBeforeText);
			if (!body.empty())
				return std::string(body);
		}

		if (LooksPackedAss(in)) {
			const auto body = AfterCommas(in, PackedAssFieldsBeforeText);
			if (!body.empty())
				return std::string(body);
		}
		return std::string(in);
	}

	std::string StripAssTags(std::string_view in) noexcept {
		std::string out;
		out.reserve(in.size());
		bool tag = false;
		for (const char c : in) {
			if (c == '{')
				tag = true;
			else if (c == '}')
				tag = false;
			else if (!tag)
				out.push_back(c);
		}
		return out;
	}

	std::string NewlinesFromAss(std::string text) noexcept {
		for (std::size_t i = 0; i + 1 < text.size(); ++i) {
			if (text[i] == '\\' && (text[i + 1] == 'N' || text[i + 1] == 'n')) {
				text[i] = '\n';
				text.erase(i + 1, 1);
			}
		}
		return text;
	}

	std::string NewlinesToAss(std::string text) noexcept {
		std::string out;
		out.reserve(text.size() + 8);
		for (const char c : text) {
			if (c == '\r')
				continue;
			if (c == '\n') {
				out += "\\N";
				continue;
			}
			out.push_back(c);
		}
		return out;
	}

	bool WantsAssRect(std::string_view impl) noexcept {
		return impl == "ass" || impl == "ssa";
	}

	bool PlainTextDest(std::string_view impl) noexcept {
		return impl == "srt" || impl == "subrip" || impl == "webvtt" || impl == "text";
	}

	std::string ReadCue(Frame& frame) noexcept {
		auto& pay = frame.Payload();
		const auto n = pay.AvailableBytes();
		if (n == 0)
			return {};
		StormByte::Buffer::DataType bytes;
		if (!pay.Peek(n, bytes) || bytes.empty())
			return {};
		const auto* raw = reinterpret_cast<const std::uint8_t*>(bytes.data());
		if (bytes.size() >= 4 && raw[0] == 'O' && raw[1] == 'C' && raw[2] == 'R' && raw[3] == '1')
			return {};
		std::size_t begin = 0;
		std::size_t end = bytes.size();
		while (begin < end && bytes[begin] == std::byte{0})
			++begin;
		while (end > begin && (bytes[end - 1] == std::byte{0} || bytes[end - 1] == std::byte{'\n'}
			|| bytes[end - 1] == std::byte{'\r'}))
			--end;
		if (begin >= end)
			return {};
		return std::string(reinterpret_cast<const char*>(bytes.data() + begin), end - begin);
	}

	std::optional<StormByte::Multimedia::OCR::GrayBitmap> ReadOcrBitmap(Frame& frame) noexcept {
		auto& pay = frame.Payload();
		const auto n = pay.AvailableBytes();
		if (n < 12)
			return std::nullopt;
		StormByte::Buffer::DataType bytes;
		if (!pay.Peek(n, bytes) || bytes.size() < 12)
			return std::nullopt;
		const auto* p = reinterpret_cast<const std::uint8_t*>(bytes.data());
		if (p[0] != 'O' || p[1] != 'C' || p[2] != 'R' || p[3] != '1')
			return std::nullopt;
		const auto get32 = [](const std::uint8_t* d) {
			return static_cast<int>(d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24));
		};
		StormByte::Multimedia::OCR::GrayBitmap out;
		out.width = get32(p + 4);
		out.height = get32(p + 8);
		out.stride = out.width;
		if (out.width <= 0 || out.height <= 0)
			return std::nullopt;
		const std::size_t need = static_cast<std::size_t>(out.width) * static_cast<std::size_t>(out.height);
		if (bytes.size() < 12 + need)
			return std::nullopt;
		out.pixels.assign(p + 12, p + 12 + need);
		return out;
	}

	std::string TessLanguage(std::string_view tag) noexcept {
		std::string out;
		for (unsigned char c : tag)
			out.push_back(static_cast<char>(std::tolower(c)));
		if (out == "es" || out == "spa")
			return "spa";
		if (out == "en" || out == "eng")
			return "eng";
		if (out == "pt" || out == "por")
			return "por";
		if (out == "fr" || out == "fra" || out == "fre")
			return "fra";
		if (out == "de" || out == "deu" || out == "ger")
			return "deu";
		if (out == "it" || out == "ita")
			return "ita";
		return out;
	}

	std::string FormatAssTime(std::int64_t nanoseconds) noexcept {
		if (nanoseconds < 0)
			nanoseconds = 0;
		const auto cs = static_cast<std::int64_t>(nanoseconds / 10000000);
		const auto h = cs / 360000;
		const auto m = (cs / 6000) % 60;
		const auto s = (cs / 100) % 60;
		const auto c = cs % 100;
		char buf[32];
		std::snprintf(buf, sizeof(buf), "%d:%02d:%02d.%02d",
			static_cast<int>(h), static_cast<int>(m), static_cast<int>(s), static_cast<int>(c));
		return buf;
	}

	std::string WrapAss(std::string text, std::int64_t startNs, std::int64_t endNs) noexcept {
		if (text.find("Dialogue:") != std::string::npos)
			return text;
		return "Dialogue: 0," + FormatAssTime(startNs) + "," + FormatAssTime(endNs)
			+ ",Default,NTP,0000,0000,0000,," + NewlinesToAss(std::move(text));
	}

	void StampSubtitlePacket(FFmpeg::AVPacket& pkt, std::int64_t pts, std::uint32_t durationMs, AVRational tb) noexcept {
		const std::int64_t duration = av_rescale_q(static_cast<std::int64_t>(durationMs), AVRational{1, 1000}, tb);
		const std::int64_t scaled = (pts == AV_NOPTS_VALUE)
			? AV_NOPTS_VALUE
			: av_rescale_q(pts, AVRational{1, AV_TIME_BASE}, tb);
		pkt.Timestamps(scaled, scaled, duration);
	}
}

Encoder::Encoder(int output_index, const StormByte::Multimedia::Codec& codec) noexcept
: m_index(output_index), m_codec(&codec), m_failed(false) {}

Encoder::Encoder(Encoder&&) noexcept = default;
Encoder::~Encoder() noexcept = default;
Encoder& Encoder::operator=(Encoder&&) noexcept = default;

Encoder::operator bool() const noexcept {
	return !m_failed && static_cast<bool>(m_impl);
}

int Encoder::Index() const noexcept {
	return m_index;
}

const StormByte::Multimedia::Codec& Encoder::Destination() const noexcept {
	return *m_codec;
}

bool Encoder::Failed() const noexcept {
	return m_failed;
}

const std::optional<std::string>& Encoder::Error() const noexcept {
	return m_error;
}

const std::optional<std::string>& Encoder::Implementation() const noexcept {
	return m_implementation;
}

void Encoder::Implementation(std::string name) noexcept {
	if (name.empty())
		m_implementation.reset();
	else
		m_implementation = std::move(name);
}

const StormByte::Multimedia::Features& Encoder::Require() const noexcept {
	return m_require;
}

void Encoder::Require(StormByte::Multimedia::Features features) noexcept {
	m_require = features;
}

const StormByte::Multimedia::Features& Encoder::Capabilities() const noexcept {
	return m_capabilities;
}

void Encoder::CRF(int value) noexcept {
	if (m_failed)
		return;
	if (m_codec->Type() != StormByte::Multimedia::Type::Video) {
		Fail("CRF is not valid for this codec");
		return;
	}
	if (m_bitRate.has_value()) {
		Fail("CRF cannot be combined with BitRate");
		return;
	}
	m_crf = value;
}

const std::optional<int>& Encoder::CRF() const noexcept {
	return m_crf;
}

void Encoder::BitRate(std::int64_t bits_per_second) noexcept {
	if (m_failed)
		return;
	if (bits_per_second <= 0) {
		Fail("BitRate must be positive");
		return;
	}
	if (m_crf.has_value()) {
		Fail("BitRate cannot be combined with CRF");
		return;
	}
	if (m_maxBitRate.has_value() && bits_per_second > *m_maxBitRate) {
		Fail("BitRate exceeds MaxBitRate");
		return;
	}
	m_bitRate = bits_per_second;
}

const std::optional<std::int64_t>& Encoder::BitRate() const noexcept {
	return m_bitRate;
}

void Encoder::MaxBitRate(std::int64_t bits_per_second) noexcept {
	if (m_failed)
		return;
	if (m_codec->Type() != StormByte::Multimedia::Type::Video) {
		Fail("MaxBitRate is not valid for this codec");
		return;
	}
	if (bits_per_second <= 0) {
		Fail("MaxBitRate must be positive");
		return;
	}
	if (m_bitRate.has_value() && *m_bitRate > bits_per_second) {
		Fail("BitRate exceeds MaxBitRate");
		return;
	}
	m_maxBitRate = bits_per_second;
}

const std::optional<std::int64_t>& Encoder::MaxBitRate() const noexcept {
	return m_maxBitRate;
}

void Encoder::Preset(std::string name) noexcept {
	if (m_failed)
		return;
	if (name.empty()) {
		m_preset.reset();
		return;
	}
	m_preset = std::move(name);
}

const std::optional<std::string>& Encoder::Preset() const noexcept {
	return m_preset;
}

void Encoder::Tune(std::string name) noexcept {
	if (m_failed)
		return;
	if (m_codec->Type() != StormByte::Multimedia::Type::Video) {
		Fail("Tune is not valid for this codec");
		return;
	}
	if (name.empty()) {
		m_tune.reset();
		return;
	}
	m_tune = std::move(name);
}

const std::optional<std::string>& Encoder::Tune() const noexcept {
	return m_tune;
}

const std::map<std::string, std::string>& Encoder::FineTune() const noexcept {
	return m_fineTune;
}

void Encoder::FineTune(std::map<std::string, std::string> options) noexcept {
	m_fineTune = std::move(options);
}

void Encoder::Flush() noexcept {
	if (m_failed || !m_impl)
		return;
	m_impl->m_encoder.SetEof();
}

void Encoder::Fail(std::string reason) noexcept {
	m_failed = true;
	m_error = std::move(reason);
	m_capabilities = StormByte::Multimedia::Features{};
	m_impl.reset();
}

void Encoder::Bind(std::unique_ptr<Impl> impl) noexcept {
	m_impl = std::move(impl);
	m_failed = false;
	m_error.reset();
}

bool Encoder::DrainOne() noexcept {
	if (m_failed || !m_impl)
		return false;
	if (m_impl->m_encoder.IsSubtitle())
		return false;
	const auto result = m_impl->m_encoder.ReceivePacket(m_impl->m_scratch);
	if (result == FFmpeg::OperationResult::TryAgain || result == FFmpeg::OperationResult::EndOfFile)
		return false;
	if (result != FFmpeg::OperationResult::Success) {
		Fail("failed to receive packet");
		return false;
	}
	m_impl->m_pending.push_back(MakePacket(m_index, m_impl->m_scratch, m_impl->m_timeBase));
	m_impl->m_scratch.Unref();
	return true;
}

bool Encoder::Open(const Frame& frame) noexcept {
	if (!m_codec->HasAccess(StormByte::Multimedia::Operation::Write)) {
		Fail("codec is not writable");
		return false;
	}
	const auto kind = m_codec->Type();
	if (kind == StormByte::Multimedia::Type::Video && !frame.Video()) {
		Fail("encoder destination is video but frame is not");
		return false;
	}
	if (kind == StormByte::Multimedia::Type::Audio && !frame.Audio()) {
		Fail("encoder destination is audio but frame is not");
		return false;
	}
	if (kind != StormByte::Multimedia::Type::Subtitle
		&& (!frame.m_impl || !frame.m_impl->m_backend.Get())) {
		Fail("frame has no backend buffer");
		return false;
	}

	const bool hdr10 = frame.Video() && frame.Video()->HDR10().has_value();
	const bool hdr10plus = hdr10 && frame.Video()->HDR10()->IsHDR10Plus();
	const auto need = FrameNeed(m_require, frame);
	const std::string stormName{m_codec->Name()};
	const std::string_view pin = m_implementation
		? std::string_view{*m_implementation} : std::string_view{};

	if (!pin.empty()) {
		if (avcodec_find_encoder_by_name(std::string(pin).c_str()) == nullptr) {
			Fail("encoder implementation is unavailable");
			return false;
		}
		const auto* listed = FindRow(stormName, pin);
		if (!listed || !listed->features.Has(need)) {
			Fail("encoder implementation lacks required features");
			return false;
		}
	}

	const auto* row = PickEncoder(stormName, pin, need);
	const ::AVCodec* codec = nullptr;
	if (row)
		codec = avcodec_find_encoder_by_name(row->name);
	if (!codec) {
		Fail("no encoder for destination codec");
		return false;
	}

	if (m_crf && (!row || !HasKey(row->crf_key))) {
		Fail("encoder implementation does not support CRF");
		return false;
	}
	if (m_bitRate && (!row || !HasKey(row->bitrate_key))) {
		Fail("encoder implementation does not support BitRate");
		return false;
	}
	if (m_maxBitRate && (!row || !HasKey(row->maxrate_key))) {
		Fail("encoder implementation does not support MaxBitRate");
		return false;
	}
	if (m_preset && (!row || !HasKey(row->preset_key))) {
		Fail("encoder implementation does not support Preset");
		return false;
	}
	if (m_tune && (!row || !HasKey(row->style_key))) {
		Fail("encoder implementation does not support Tune");
		return false;
	}

	for (const auto& [key, value] : m_fineTune) {
		(void)value;
		if (key == "bufsize" || key == "vbv-bufsize") {
			Fail("FineTune cannot set bufsize");
			return false;
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

	if (m_crf && row && HasKey(row->crf_key))
		opts.emplace(row->crf_key, std::to_string(*m_crf));
	if (m_bitRate && row && HasKey(row->bitrate_key))
		opts.emplace(row->bitrate_key, std::to_string(*m_bitRate));
	if (m_maxBitRate && row && HasKey(row->maxrate_key))
		opts.emplace(row->maxrate_key, std::to_string(*m_maxBitRate));
	if (row && HasKey(row->bufsize_key) && frame.Video())
		opts.emplace(row->bufsize_key, std::to_string(BufSizeBits(*this, frame)));
	if (m_preset && row && HasKey(row->preset_key))
		opts.emplace(row->preset_key, *m_preset);
	if (m_tune && row && HasKey(row->style_key))
		opts.emplace(row->style_key, *m_tune);

	const bool pack = row && HasKey(row->tune_key);
	for (const auto& [key, value] : m_fineTune) {
		if (key.empty())
			continue;
		if (blob.contains(key) && blob[key] != value) {
			Fail("FineTune conflicts with HDR signaling key '" + key + "'");
			return false;
		}
		if (opts.contains(key) && opts[key] != value) {
			Fail("FineTune conflicts with encoder setter key '" + key + "'");
			return false;
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
	if (frame.Audio() && frame.m_impl && frame.m_impl->m_backend.Get())
		sampleFormat = frame.m_impl->m_backend.Get()->format;

	auto params = FillParams(frame, codec, m_bitRate, sampleFormat);
	const auto timeBase = ChooseTimeBase(frame, kind);
	auto backend = FFmpeg::AVEncoder::Open(const_cast<::AVCodec*>(codec), params, m_index, opts, timeBase);
	if (!backend.has_value()) {
		Fail(backend.error()->what());
		return false;
	}

	auto impl = std::make_unique<Impl>(std::move(backend.value()));
	impl->m_timeBase = impl->m_encoder.TimeBase();
	if (impl->m_timeBase.num <= 0 || impl->m_timeBase.den <= 0)
		impl->m_timeBase = timeBase;
	if (row) {
		Implementation(row->name);
		m_capabilities = row->features;
	}
	Bind(std::move(impl));
	return true;
}

Frame& StormByte::Multimedia::Pipeline::operator>>(Frame& frame, Encoder& encoder) noexcept {
	if (encoder.m_failed)
		return frame;
	if (!encoder.m_impl && !encoder.Open(frame))
		return frame;
	if (!encoder.m_impl)
		return frame;

	if (encoder.m_codec->Type() == StormByte::Multimedia::Type::Subtitle) {
		auto text = DialogueBody(ReadCue(frame));
		if (text.empty()) {
			auto bitmap = ReadOcrBitmap(frame);
			if (!bitmap)
				return frame;
			if (frame.Language())
				encoder.m_impl->m_ocr.Language(TessLanguage(*frame.Language()));
			else
				encoder.m_impl->m_ocr.Language({});
			auto recognized = encoder.m_impl->m_ocr.Recognize(
				std::span<const std::uint8_t>(bitmap->pixels.data(), bitmap->pixels.size()),
				bitmap->width,
				bitmap->height,
				bitmap->stride);
			if (!recognized.has_value()) {
				encoder.Fail(recognized.error()->what());
				return frame;
			}
			text = std::move(recognized.value());
			if (text.empty())
				return frame;
		}
		const auto impl = encoder.m_implementation ? *encoder.m_implementation : std::string{};
		const std::int64_t startNs = frame.Pts() ? frame.Pts()->Nanoseconds().count() : 0;
		std::int64_t durationNs = 0;
		if (frame.Duration())
			durationNs = frame.Duration()->Nanoseconds().count();
		if (durationNs < 0)
			durationNs = 0;
		const std::int64_t endNs = startNs + durationNs;
		const std::int64_t pts = frame.Pts()
			? NsToTicks(startNs, AVRational{1, AV_TIME_BASE})
			: AV_NOPTS_VALUE;
		std::uint32_t durationMs = 0;
		if (durationNs > 0) {
			const auto ms = durationNs / 1000000;
			if (ms > 0)
				durationMs = static_cast<std::uint32_t>(ms);
		}
		const AVRational tb = (encoder.m_impl->m_timeBase.num > 0)
			? encoder.m_impl->m_timeBase
			: AVRational{1, AV_TIME_BASE};

		if (PlainTextDest(impl)) {
			text = NewlinesFromAss(StripAssTags(text));
			if (!encoder.m_impl->m_scratch.Load(
					reinterpret_cast<const std::uint8_t*>(text.data()),
					static_cast<int>(text.size()),
					encoder.m_index, true)) {
				encoder.Fail("failed to encode subtitle");
				return frame;
			}
			StampSubtitlePacket(encoder.m_impl->m_scratch, pts, durationMs, tb);
			encoder.m_impl->m_pending.push_back(
				MakePacket(encoder.m_index, encoder.m_impl->m_scratch, encoder.m_impl->m_timeBase));
			encoder.m_impl->m_scratch.Unref();
			return frame;
		}

		if (WantsAssRect(impl))
			text = WrapAss(std::move(text), startNs, endNs);
		FFmpeg::AVSubtitle sub;
		sub.FillText(std::move(text), pts, durationMs, WantsAssRect(impl));
		const auto result = encoder.m_impl->m_encoder.EncodeSubtitle(sub, encoder.m_impl->m_scratch);
		if (result != FFmpeg::OperationResult::Success) {
			encoder.Fail("failed to encode subtitle");
			return frame;
		}
		encoder.m_impl->m_pending.push_back(
			MakePacket(encoder.m_index, encoder.m_impl->m_scratch, encoder.m_impl->m_timeBase));
		encoder.m_impl->m_scratch.Unref();
		return frame;
	}

	if (!frame.m_impl || !frame.m_impl->m_backend.Get()) {
		encoder.Fail("frame has no backend buffer");
		return frame;
	}

	if (frame.Video() && frame.Video()->HDR10()) {
		if (frame.Video()->HDR10()->IsHDR10Plus()
			&& !encoder.m_capabilities.Has(StormByte::Multimedia::Feature::HDR10Plus)) {
			encoder.Fail("encoder implementation lacks required features");
			return frame;
		}
		frame.m_impl->m_backend.WriteHdr10(*frame.Video()->HDR10());
	}
	frame.m_impl->m_backend.WriteSideData(frame.Attachments());

	auto* raw = frame.m_impl->m_backend.Get();
	const auto tb = encoder.m_impl->m_timeBase;
	if (frame.Pts())
		raw->pts = NsToTicks(frame.Pts()->Nanoseconds().count(), tb);
	if (frame.Duration())
		raw->duration = NsToTicks(frame.Duration()->Nanoseconds().count(), tb);

	auto result = encoder.m_impl->m_encoder.SendFrame(frame.m_impl->m_backend);
	while (result == FFmpeg::OperationResult::TryAgain) {
		if (!encoder.DrainOne()) {
			if (encoder.m_failed)
				return frame;
			encoder.Fail("encoder stalled");
			return frame;
		}
		result = encoder.m_impl->m_encoder.SendFrame(frame.m_impl->m_backend);
	}
	if (result == FFmpeg::OperationResult::Error)
		encoder.Fail("failed to send frame");
	return frame;
}

Encoder& StormByte::Multimedia::Pipeline::operator>>(Encoder& encoder, Packet& packet) noexcept {
	if (encoder.m_failed || !encoder.m_impl)
		return encoder;
	if (!encoder.m_impl->m_pending.empty()) {
		packet = std::move(encoder.m_impl->m_pending.front());
		encoder.m_impl->m_pending.pop_front();
		return encoder;
	}
	if (!encoder.DrainOne())
		return encoder;
	packet = std::move(encoder.m_impl->m_pending.front());
	encoder.m_impl->m_pending.pop_front();
	return encoder;
}
