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

#include <StormByte/multimedia/backend/ffmpeg/AVSubtitle.hxx>
#include <StormByte/multimedia/ocr/engine.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/details/subtitle.hxx>
#include <StormByte/multimedia/pipeline/engine/encoder/open.hxx>
#include <StormByte/multimedia/pipeline/engine/frame/engine.hxx>
#include <StormByte/multimedia/type.hxx>

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
	#include <libavutil/mathematics.h>
	#include <libavutil/rational.h>
}

namespace FFmpeg = StormByte::Multimedia::Backend::FFmpeg;
namespace Open = StormByte::Multimedia::Pipeline::Engine::Encoder::Open;
using StormByte::Multimedia::Type;

namespace {
	constexpr std::size_t DialogueFieldsBeforeText = 9;
	constexpr std::size_t PackedAssFieldsBeforeText = 8;
	constexpr std::int64_t FallbackCueNs = 2000000000; // 2 s if Flush with no hide packet

	struct GrayBitmap {
		int width = 0;
		int height = 0;
		int stride = 0;
		std::vector<std::uint8_t> pixels;
	};

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

	std::string ReadCue(class StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
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

	std::optional<GrayBitmap> ReadOcrBitmap(class StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
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
		GrayBitmap out;
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

	void DbgSub(const char* what, std::int64_t startNs, std::uint32_t durMs, const std::string& text) noexcept {
		std::string one = text;
		for (char& c : one) {
			if (c == '\n' || c == '\r')
				c = ' ';
		}
		if (one.size() > 80)
			one.resize(80);
		std::fprintf(stderr, "STMM-S %s start_ns=%lld dur_ms=%u ocr=\"%s\"\n",
			what,
			static_cast<long long>(startNs),
			durMs,
			one.c_str());
		std::fflush(stderr);
	}
}

StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Subtitle::Subtitle() noexcept
: m_timeBase{1, AV_TIME_BASE},
m_index(0),
m_flushed(false),
m_heldStartNs(0),
m_heldPts(AV_NOPTS_VALUE) {}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Subtitle::IsOpen() const noexcept {
	return m_encoder.has_value();
}

const AVCodecContext* StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Subtitle::Context() const noexcept {
	return m_encoder ? m_encoder->Get() : nullptr;
}

AVRational StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Subtitle::TimeBase() const noexcept {
	return m_timeBase;
}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Subtitle::Open(
	class StormByte::Multimedia::Pipeline::Encoder& owner,
	const class StormByte::Multimedia::Pipeline::Frame& frame) noexcept {
	if (m_encoder)
		return true;
	if (frame.Type() != StormByte::Multimedia::Type::Subtitle) {
		owner.Fail("encoder destination is subtitle but frame is not");
		return false;
	}
	auto opened = Open::OpenBackend(owner, frame);
	if (!opened)
		return false;
	m_timeBase = opened->timeBase;
	if (m_timeBase.num <= 0 || m_timeBase.den <= 0)
		m_timeBase = AVRational{1, AV_TIME_BASE};
	if (!opened->implementation.empty())
		owner.Implementation(opened->implementation);
	owner.m_capabilities = opened->capabilities;
	m_encoder = std::move(opened->encoder);
	m_index = owner.Index();
	return true;
}

void StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Subtitle::EmitHeld(
	class StormByte::Multimedia::Pipeline::Encoder& owner,
	std::int64_t endNs) noexcept {
	if (m_heldText.empty() || m_heldPts == AV_NOPTS_VALUE)
		return;

	std::int64_t durationNs = (endNs > m_heldStartNs) ? (endNs - m_heldStartNs) : 0;
	if (durationNs <= 0)
		durationNs = FallbackCueNs;
	auto durationMs = static_cast<std::uint32_t>(durationNs / 1000000);
	if (durationMs == 0)
		durationMs = 1;

	const AVRational tb = (m_timeBase.num > 0) ? m_timeBase : AVRational{1, AV_TIME_BASE};
	const auto impl = owner.Implementation() ? *owner.Implementation() : std::string{};
	std::string text = m_heldText;
	DbgSub("emit", m_heldStartNs, durationMs, text);

	if (PlainTextDest(impl)) {
		text = NewlinesFromAss(StripAssTags(text));
		if (!m_scratch.Load(
				reinterpret_cast<const std::uint8_t*>(text.data()),
				static_cast<int>(text.size()),
				owner.Index(), true)) {
			owner.Fail("failed to encode subtitle");
			m_heldText.clear();
			m_heldPts = AV_NOPTS_VALUE;
			return;
		}
		StampSubtitlePacket(m_scratch, m_heldPts, durationMs, tb);
		m_pending.push_back(Open::MakePacket(Type::Subtitle, owner.Index(), m_scratch, m_timeBase, true));
		m_scratch.Unref();
	}
	else {
		if (WantsAssRect(impl))
			text = WrapAss(std::move(text), m_heldStartNs, m_heldStartNs + durationNs);
		FFmpeg::AVSubtitle sub;
		sub.FillText(std::move(text), m_heldPts, durationMs, WantsAssRect(impl));
		if (!m_encoder || m_encoder->EncodeSubtitle(sub, m_scratch) != FFmpeg::OperationResult::Success) {
			owner.Fail("failed to encode subtitle");
			m_heldText.clear();
			m_heldPts = AV_NOPTS_VALUE;
			return;
		}
		m_pending.push_back(Open::MakePacket(Type::Subtitle, owner.Index(), m_scratch, m_timeBase, true));
		m_scratch.Unref();
	}

	m_heldText.clear();
	m_heldPts = AV_NOPTS_VALUE;
}

bool StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Subtitle::Push(
	class StormByte::Multimedia::Pipeline::Encoder& owner,
	const std::shared_ptr<StormByte::Multimedia::Pipeline::Frame>& frame) noexcept {
	if (!frame) {
		owner.Fail("empty frame");
		return false;
	}
	if (!m_encoder && !Open(owner, *frame))
		return false;
	if (!m_encoder)
		return false;

	auto text = DialogueBody(ReadCue(*frame));
	if (text.empty()) {
		auto bitmap = ReadOcrBitmap(*frame);
		if (!bitmap)
			return true;
		if (frame->Language())
			m_ocr.Language(TessLanguage(*frame->Language()));
		else
			m_ocr.Language({});
		auto recognized = m_ocr.Recognize(
			std::span<const std::uint8_t>(bitmap->pixels.data(), bitmap->pixels.size()),
			bitmap->width,
			bitmap->height,
			bitmap->stride);
		if (!recognized.has_value()) {
			owner.Fail(recognized.error() ? recognized.error()->what() : "OCR failed");
			return false;
		}
		text = std::move(recognized.value());
		if (text.empty())
			return true;
	}

	const std::int64_t startNs = frame->Pts() ? frame->Pts()->Nanoseconds().count() : 0;
	std::int64_t durationNs = 0;
	if (frame->Duration())
		durationNs = frame->Duration()->Nanoseconds().count();
	if (durationNs < 0)
		durationNs = 0;
	const std::int64_t pts = frame->Pts()
		? Open::NsToTicks(startNs, AVRational{1, AV_TIME_BASE})
		: AV_NOPTS_VALUE;

	DbgSub("in", startNs, static_cast<std::uint32_t>(durationNs / 1000000), text);

	if (!m_heldText.empty())
		EmitHeld(owner, startNs);
	if (owner.Failed())
		return false;

	m_heldText = std::move(text);
	m_heldStartNs = startNs;
	m_heldPts = pts;
	if (durationNs > 0)
		EmitHeld(owner, startNs + durationNs);
	return !owner.Failed();
}

void StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Subtitle::Flush(
	class StormByte::Multimedia::Pipeline::Encoder& owner) noexcept {
	if (!m_heldText.empty())
		EmitHeld(owner, m_heldStartNs + FallbackCueNs);
	m_flushed = true;
}

std::shared_ptr<StormByte::Multimedia::Pipeline::Packet>
StormByte::Multimedia::Pipeline::Engine::Encoder::Details::Subtitle::Take() noexcept {
	if (m_pending.empty())
		return {};
	auto packet = std::move(m_pending.front());
	m_pending.pop_front();
	return packet;
}
