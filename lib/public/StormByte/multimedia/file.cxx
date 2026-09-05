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

#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/detail/probe.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVStream.hxx>
#include <StormByte/multimedia/registry.hxx>

#include <fstream>
#include <string>
#include <string_view>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia;
namespace FFmpeg = StormByte::Multimedia::Engine::Backend::FFmpeg;

namespace {
	ExpectedContainer ResolveContainer(std::string_view formatName) noexcept {
		auto& registry = Registry::Instance();
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
}

ExpectedFile File::Open(const std::filesystem::path& path) noexcept {
	std::string reason;
	if (!IsReadableFile(path, reason))
		return Unexpected(FileOpenErrorException(path.string(), reason));

	auto opened = FFmpeg::AVFormatContext::Open(path);
	if (!opened.has_value())
		return Unexpected(FileOpenErrorException(path.string(), opened.error()->what()));

	const FFmpeg::AVFormatContext& ctx = opened.value();
	const char* formatName = ctx.FormatName();
	if (!formatName)
		return Unexpected(FileOpenErrorException(path.string(), "unknown container format"));

	auto container = ResolveContainer(formatName);
	if (!container.has_value())
		return Unexpected(FileOpenErrorException(path.string(), container.error()->what()));

	Multimedia::Streams streams;
	for (const auto& stream : ctx.Streams()) {
		auto codec = ResolveCodec(stream);
		if (!codec.has_value())
			return Unexpected(FileOpenErrorException(path.string(), codec.error()->what()));
		streams.emplace_back(Stream(codec.value(), Detail::Probe::Stream(stream)));
	}

	return File(path, container.value(), std::move(streams), Detail::Probe::File(ctx));
}
