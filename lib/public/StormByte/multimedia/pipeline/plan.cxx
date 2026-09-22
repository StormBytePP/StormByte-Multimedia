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

#include <StormByte/multimedia/pipeline/plan.hxx>

#include <StormByte/expected.hxx>
#include <StormByte/multimedia/backend/pipeline/detail/cover.hxx>
#include <StormByte/multimedia/codec.hxx>
#include <StormByte/multimedia/pipeline/config/attachment.hxx>
#include <StormByte/multimedia/pipeline/config/audio.hxx>
#include <StormByte/multimedia/pipeline/config/base.hxx>
#include <StormByte/multimedia/pipeline/config/subtitle.hxx>
#include <StormByte/multimedia/pipeline/config/video.hxx>
#include <StormByte/multimedia/pipeline/demuxer.hxx>
#include <StormByte/multimedia/pipeline/exception.hxx>
#include <StormByte/multimedia/registry.hxx>
#include <StormByte/multimedia/type.hxx>

#include <cctype>
#include <string>
#include <utility>

using StormByte::Buffer::IO::BufferedFileReader;
using StormByte::Buffer::IO::BufferedFileWriter;
using namespace StormByte::Multimedia::Pipeline;

namespace {
	std::string ExtensionOf(const std::filesystem::path& path) noexcept {
		std::string ext = path.extension().string();
		if (ext.empty())
			return {};
		if (ext.front() == '.')
			ext.erase(ext.begin());
		for (char& ch : ext)
			ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
		return ext;
	}

	const StormByte::Multimedia::Codec* DestinationCodec(
		const StormByte::Multimedia::Pipeline::Config::Base* config) noexcept {
		if (!config)
			return nullptr;
		if (const auto* video = dynamic_cast<const Config::Video*>(config))
			return video->Codec();
		if (const auto* audio = dynamic_cast<const Config::Audio*>(config))
			return audio->Codec();
		if (const auto* sub = dynamic_cast<const Config::Subtitle*>(config))
			return sub->Codec();
		return nullptr;
	}

	std::optional<std::string> AttachmentMime(
		const StormByte::Multimedia::Pipeline::Config::Base* config) noexcept {
		if (!config)
			return std::nullopt;
		if (const auto* att = dynamic_cast<const Config::Attachment*>(config))
			return att->MimeType();
		return std::nullopt;
	}
}

Plan::Plan(const std::filesystem::path& source,
	const std::filesystem::path& destination) noexcept
: Plan(BufferedFileReader{source}, BufferedFileWriter{destination}) {}

Plan::Plan(std::unique_ptr<BufferedFileReader> reader,
	std::unique_ptr<BufferedFileWriter> writer) noexcept
: m_reader(std::move(reader)),
	m_writer(std::move(writer)),
	m_container(m_writer ? ContainerFromWriter(*m_writer) : nullptr) {
	if (!m_reader)
		return;
	auto opened = StormByte::Multimedia::File::Open(*m_reader);
	if (opened)
		m_snapshot.emplace(std::move(*opened));
}

const StormByte::Multimedia::Container* Plan::ContainerFromWriter(
	const BufferedFileWriter& writer) noexcept {
	const auto ext = ExtensionOf(writer.Path());
	if (ext.empty())
		return nullptr;
	auto found = StormByte::Multimedia::Registry::Instance().FindContainer(ext);
	if (!found)
		return nullptr;
	return &found.value().get();
}

BufferedFileReader& Plan::Reader() noexcept {
	return *m_reader;
}

const BufferedFileReader& Plan::Reader() const noexcept {
	return *m_reader;
}

BufferedFileWriter& Plan::Writer() noexcept {
	return *m_writer;
}

const BufferedFileWriter& Plan::Writer() const noexcept {
	return *m_writer;
}

const StormByte::Multimedia::File& Plan::Snapshot() const noexcept {
	return *m_snapshot;
}

Plan::operator bool() const noexcept {
	return Check().has_value();
}

CheckResult Plan::Check() const {
	if (!m_reader || !m_writer)
		return StormByte::Unexpected<PlanException>("plan was moved-from");
	if (!m_snapshot)
		return StormByte::Unexpected<PlanException>("source snapshot failed");
	if (m_writer->Path().empty())
		return StormByte::Unexpected<PlanException>("destination path is empty");
	if (!m_container)
		return StormByte::Unexpected<PlanException>("destination container is unknown");
	if (m_tracks.empty())
		return StormByte::Unexpected<PlanException>("plan has no tracks");

	for (const auto& held : m_tracks) {
		if (!held)
			return StormByte::Unexpected<PlanException>("plan holds an empty track");
		const Track& track = *held;
		if (track.In() < 0)
			return StormByte::Unexpected<PlanException>("track origin index is negative");
		if (track.Type() == StormByte::Multimedia::Type::Unknown)
			return StormByte::Unexpected<PlanException>("track type is unknown");

		if (track.Type() == StormByte::Multimedia::Type::Attachment) {
			const auto mime = AttachmentMime(track.Config());
			if (!mime || mime->empty())
				return StormByte::Unexpected<PlanException>("attachment MIME is empty");
			if (!Detail::MimePatternOk(*mime))
				return StormByte::Unexpected<PlanException>("attachment MIME is not exact, type-star or star-star");
			continue;
		}

		const auto* dest = DestinationCodec(track.Config());
		if (dest && dest->Type() != track.Type())
			return StormByte::Unexpected<PlanException>("destination codec type does not match the track");
	}

	return CheckResult{};
}

Demuxer& StormByte::Multimedia::Pipeline::operator>>(Plan&& plan, Demuxer& demuxer) noexcept {
	if (demuxer.Plan()) {
		demuxer.Fail("demuxer already has a plan");
		return demuxer;
	}

	Step& step = demuxer;
	step.m_plan = std::make_shared<Plan>(std::move(plan));
	demuxer.m_planPresent.notify_all();
	demuxer.Wake().notify_all();
	return demuxer;
}
