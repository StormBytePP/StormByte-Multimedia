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
#include <StormByte/multimedia/pipeline/config/attachment.hxx>
#include <StormByte/multimedia/pipeline/config/audio.hxx>
#include <StormByte/multimedia/pipeline/config/subtitle.hxx>
#include <StormByte/multimedia/pipeline/config/video.hxx>
#include <StormByte/multimedia/pipeline/demux.hxx>
#include <StormByte/multimedia/pipeline/exception.hxx>
#include <StormByte/multimedia/stream.hxx>
#include <StormByte/multimedia/type.hxx>

using namespace StormByte::Multimedia::Pipeline;
using StormByte::Multimedia::Type;

namespace {
	const StormByte::Multimedia::Codec* DestinationCodec(const Config::Base* config) noexcept {
		if (const auto* video = dynamic_cast<const Config::Video*>(config))
			return video->Codec();
		if (const auto* audio = dynamic_cast<const Config::Audio*>(config))
			return audio->Codec();
		if (const auto* subtitle = dynamic_cast<const Config::Subtitle*>(config))
			return subtitle->Codec();
		return nullptr;
	}

	bool MimeHasWildcard(const std::string& mime) noexcept {
		return mime.find('*') != std::string::npos;
	}
}

Plan::Plan(StormByte::Multimedia::File&& source,
	const StormByte::Multimedia::Container& container,
	std::filesystem::path destination) noexcept
: m_source(std::make_unique<StormByte::Multimedia::File>(std::move(source))),
m_container(&container),
m_destination(std::move(destination)) {}

CheckResult Plan::Check() const {
	if (!m_source)
		return StormByte::Unexpected<PlanException>("plan has no source");
	if (m_destination.empty())
		return StormByte::Unexpected<PlanException>("destination path is empty");
	if (m_tracks.empty())
		return StormByte::Unexpected<PlanException>("plan has no tracks");

	const auto& streams = m_source->Streams();
	const auto& attachments = m_source->Attachments();

	for (const std::unique_ptr<Track>& item : m_tracks) {
		const Track& track = *item;
		const int in = track.In();
		if (in < 0)
			return StormByte::Unexpected<PlanException>("track origin {} is negative", in);

		const Type type = track.Type();
		if (type == Type::Unknown)
			return StormByte::Unexpected<PlanException>("track origin {} has unknown type", in);

		if (type == Type::Attachment) {
			if (static_cast<std::size_t>(in) >= attachments.size())
				return StormByte::Unexpected<PlanException>("attachment origin {} is not in source", in);
			if (const auto* attachment = dynamic_cast<const Config::Attachment*>(track.Config())) {
				if (attachment->MimeType().empty() || MimeHasWildcard(attachment->MimeType()))
					return StormByte::Unexpected<PlanException>("attachment origin {} has invalid MIME type", in);
			}
			continue;
		}

		const StormByte::Multimedia::Stream* stream = nullptr;
		for (const auto& candidate : streams) {
			if (candidate.Index() == in) {
				stream = &candidate;
				break;
			}
		}
		if (!stream)
			return StormByte::Unexpected<PlanException>("track origin {} is not in source", in);
		if (stream->Type() != type)
			return StormByte::Unexpected<PlanException>("track origin {} type does not match source", in);

		if (const StormByte::Multimedia::Codec* codec = DestinationCodec(track.Config())) {
			if (codec->Type() != stream->Type())
				return StormByte::Unexpected<PlanException>("track origin {} codec type does not match source", in);
		}
	}

	return {};
}

Demux& StormByte::Multimedia::Pipeline::operator>>(Plan&& plan, Demux& demux) noexcept {
	if (!demux.m_plan) {
		demux.m_plan = std::make_shared<Plan>(std::move(plan));
		demux.m_ready.notify_all();
	}
	return demux;
}
