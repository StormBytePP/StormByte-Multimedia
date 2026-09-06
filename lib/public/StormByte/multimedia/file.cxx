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

#include <StormByte/multimedia/detail/probe.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVStream.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/property.hxx>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/registry.hxx>

#include <cctype>
#include <cstdint>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/avutil.h>
}

using namespace StormByte::Multimedia;
namespace FFmpeg = StormByte::Multimedia::Engine::Backend::FFmpeg;

namespace {
	const std::filesystem::path& EmptyPath() noexcept {
		static const std::filesystem::path empty;
		return empty;
	}

	std::string LowerExt(const std::filesystem::path& path) noexcept {
		std::string ext = path.extension().string();
		if (!ext.empty() && ext.front() == '.')
			ext.erase(ext.begin());
		for (char& c : ext)
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		return ext;
	}

	bool TokenListContains(std::string_view formatName, std::string_view token) noexcept {
		std::string_view rest = formatName;
		while (!rest.empty()) {
			const auto comma = rest.find(',');
			const auto part = rest.substr(0, comma);
			if (part == token)
				return true;
			if (comma == std::string_view::npos)
				break;
			rest = rest.substr(comma + 1);
		}
		return false;
	}

	ExpectedContainer ResolveContainer(std::string_view formatName,
		const std::filesystem::path* path) noexcept {
		auto& registry = Registry::Instance();

		if (path && path->has_extension()) {
			const std::string ext = LowerExt(*path);
			if (!ext.empty()) {
				auto byExt = registry.FindContainer(ext);
				if (byExt.has_value()) {
					const bool listed = TokenListContains(formatName, ext)
						|| TokenListContains(formatName, byExt.value().get().Name());
					if (listed || ext == "opus" || ext == "spx")
						return byExt;
				}
			}
		}

		std::string_view rest = formatName;
		while (!rest.empty()) {
			const auto comma = rest.find(',');
			const auto token = rest.substr(0, comma);
			if (!token.empty()) {
				auto found = registry.FindContainer(token);
				if (found.has_value())
					return found;
			}
			if (comma == std::string_view::npos)
				break;
			rest = rest.substr(comma + 1);
		}
		return Unexpected<ContainerNotFoundException>(std::string(formatName));
	}

	ExpectedCodec ResolveCodec(const FFmpeg::AVStream& stream) noexcept {
		const auto params = stream.CodecParameters();
		const char* name = avcodec_get_name(static_cast<AVCodecID>(params.CodecId()));
		if (!name || name[0] == '\0')
			return Unexpected<CodecNotFoundException>(std::string("unknown"));
		return Registry::Instance().FindCodec(name);
	}

	bool IsReadableFile(const std::filesystem::path& path, std::string& reason) noexcept {
		std::error_code ec;
		if (!std::filesystem::exists(path, ec) || ec) {
			reason = "file does not exist";
			return false;
		}
		if (!std::filesystem::is_regular_file(path, ec) || ec) {
			reason = "path is not a regular file";
			return false;
		}
		std::ifstream in(path, std::ios::binary);
		if (!in) {
			reason = "file is not readable";
			return false;
		}
		return true;
	}

	std::string NestedReason(const char* what) noexcept {
		if (!what || what[0] == '\0')
			return "unknown error";
		const std::string_view view{what};
		const auto pos = view.rfind(": ");
		if (pos == std::string_view::npos)
			return std::string(view);
		return std::string(view.substr(pos + 2));
	}

	std::optional<std::chrono::nanoseconds> TicksToNs(std::int64_t ticks, AVRational timeBase) noexcept {
		if (ticks < 0 || timeBase.den <= 0)
			return std::nullopt;
		const std::int64_t ns = av_rescale_q(ticks, timeBase, AVRational{1, 1000000000});
		if (ns < 0)
			return std::nullopt;
		return std::chrono::nanoseconds{ns};
	}

	FFmpeg::ExpectedAVFormatContext OpenSource(File::Source& source) {
		if (auto* path = std::get_if<std::filesystem::path>(&source))
			return FFmpeg::AVFormatContext::Open(*path);
		return FFmpeg::AVFormatContext::Open(std::get<StormByte::Buffer::Consumer>(source));
	}

	std::string SourceName(const File::Source& source) {
		if (const auto* path = std::get_if<std::filesystem::path>(&source))
			return path->string();
		return "buffer";
	}
}

ExpectedFile File::Open(const std::filesystem::path& path) noexcept {
	return Open(Source{path}, std::nullopt);
}

ExpectedFile File::Open(const std::filesystem::path& path, std::chrono::nanoseconds duration) noexcept {
	return Open(Source{path}, std::optional<std::chrono::nanoseconds>{duration});
}

ExpectedFile File::Open(StormByte::Buffer::Consumer consumer) noexcept {
	return Open(Source{std::move(consumer)}, std::nullopt);
}

ExpectedFile File::Open(StormByte::Buffer::Consumer consumer, std::chrono::nanoseconds duration) noexcept {
	return Open(Source{std::move(consumer)}, std::optional<std::chrono::nanoseconds>{duration});
}

ExpectedFile File::Open(Source source, std::optional<std::chrono::nanoseconds> knownDuration) noexcept {
	if (auto* path = std::get_if<std::filesystem::path>(&source)) {
		std::string reason;
		if (!IsReadableFile(*path, reason))
			return Unexpected(FileOpenErrorException(path->string(), reason));
	}

	auto opened = OpenSource(source);
	if (!opened.has_value())
		return Unexpected(FileOpenErrorException(SourceName(source), NestedReason(opened.error()->what())));

	const FFmpeg::AVFormatContext& ctx = opened.value();
	const char* formatName = ctx.FormatName();
	if (!formatName)
		return Unexpected(FileOpenErrorException(SourceName(source), "unknown container format"));

	const auto* path = std::get_if<std::filesystem::path>(&source);
	auto container = ResolveContainer(formatName, path);
	if (!container.has_value())
		return Unexpected(FileOpenErrorException(SourceName(source), NestedReason(container.error()->what())));

	Multimedia::Streams streams;
	for (const auto& stream : ctx.Streams()) {
		auto codec = ResolveCodec(stream);
		if (!codec.has_value())
			return Unexpected(FileOpenErrorException(SourceName(source), NestedReason(codec.error()->what())));
		streams.emplace_back(Stream(
			codec.value(),
			Detail::Probe::Stream(stream),
			stream.Duration(),
			FFmpeg::MapProperties(stream)
		));
	}

	if (knownDuration.has_value())
		return File(std::move(source), container.value(), std::move(streams), Detail::Probe::File(ctx),
			knownDuration, true);

	auto header = ctx.Duration();
	return File(std::move(source), container.value(), std::move(streams), Detail::Probe::File(ctx),
		header, header.has_value());
}

const std::filesystem::path& File::Path() const noexcept {
	if (const auto* path = std::get_if<std::filesystem::path>(&m_source))
		return *path;
	return EmptyPath();
}

const std::optional<std::chrono::nanoseconds>& File::Duration() const noexcept {
	if (!m_durationResolved)
		ResolveDuration();
	return m_duration;
}

void File::ResolveDuration() const noexcept {
	m_durationResolved = true;

	auto opened = OpenSource(m_source);
	if (!opened.has_value())
		return;

	FFmpeg::AVFormatContext& ctx = opened.value();
	std::unordered_map<int, std::size_t> byIndex;
	std::vector<AVRational> timeBase;
	std::vector<std::int64_t> endTick;
	std::size_t i = 0;
	for (const auto& stream : ctx.Streams()) {
		byIndex.emplace(stream.Index(), i);
		timeBase.push_back(stream.TimeBase());
		endTick.push_back(AV_NOPTS_VALUE);
		++i;
	}

	FFmpeg::AVPacket packet;
	for (;;) {
		const auto result = ctx.ReadPacket(packet);
		if (result == FFmpeg::OperationResult::EndOfFile)
			break;
		if (result == FFmpeg::OperationResult::TryAgain)
			continue;
		if (result != FFmpeg::OperationResult::Success)
			break;

		const auto found = byIndex.find(packet.StreamIndex());
		if (found == byIndex.end())
			continue;
		std::int64_t pts = packet.Pts();
		if (pts == AV_NOPTS_VALUE)
			continue;
		const std::int64_t dur = packet.Duration();
		if (dur > 0)
			pts += dur;
		std::int64_t& end = endTick[found->second];
		if (end == AV_NOPTS_VALUE || pts > end)
			end = pts;
	}

	std::optional<std::chrono::nanoseconds> longest;
	for (std::size_t n = 0; n < endTick.size(); ++n) {
		auto ns = TicksToNs(endTick[n], timeBase[n]);
		if (!ns.has_value())
			continue;
		if (n < m_streams.size() && !m_streams[n].m_duration.has_value())
			m_streams[n].m_duration = ns;
		if (!longest.has_value() || *ns > *longest)
			longest = ns;
	}

	if (!m_duration.has_value())
		m_duration = longest;
}
