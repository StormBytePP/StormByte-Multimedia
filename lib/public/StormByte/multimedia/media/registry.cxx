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

#include <StormByte/multimedia/media/tables/codec/table.hxx>
#include <StormByte/multimedia/media/registry.hxx>

#include <string>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia::Media;

namespace {
	Access ProbeAccess(const Tables::Codec::CodecDef& def) noexcept {
		Access access(Operation::Read);
		for (std::size_t i = 0; i < def.FfmpegIdCount(); ++i) {
			const AVCodecDescriptor* desc = avcodec_descriptor_get_by_name(def.FfmpegId(i));
			if (!desc)
				continue;
			if (avcodec_find_encoder(desc->id) != nullptr) {
				access |= Access(Operation::Write);
				break;
			}
		}
		return access;
	}
}

Registry::Registry() noexcept {
	Initialize();
}

Registry& Registry::Instance() noexcept {
	static Registry instance;
	return instance;
}

CodecRefs Registry::CodecList(Type type) const noexcept {
	CodecRefs out;
	const auto it = m_by_type.find(type);
	if (it == m_by_type.end())
		return out;

	out.reserve(it->second.size());
	for (const std::size_t i : it->second)
		out.emplace_back(m_codecs[i]);
	return out;
}

ExpectedCodec Registry::FindCodec(std::string_view name) const noexcept {
	const auto it = m_by_name.find(name);
	if (it == m_by_name.end())
		return Unexpected<CodecNotFoundException>(std::string(name));

	return m_codecs[it->second];
}

void Registry::Add(Type type, const Tables::Codec::CodecDef& def) noexcept {
	const std::size_t i = m_codecs.size();
	m_codecs.push_back(Codec(type, def.name, def.description, ProbeAccess(def)));
	const Codec& stored = m_codecs[i];

	m_by_name.emplace(stored.Name(), i);
	for (std::size_t n = 0; n < def.FfmpegIdCount(); ++n)
		m_by_name.emplace(def.FfmpegId(n), i);
	m_by_type[type].push_back(i);
}

void Registry::Load(Type type, std::span<const Tables::Codec::CodecDef> table) noexcept {
	for (const Tables::Codec::CodecDef& def : table)
		Add(type, def);
}

void Registry::Initialize() noexcept {
	const auto video 		= Tables::Codec::Video();
	const auto audio 		= Tables::Codec::Audio();
	const auto subtitle 	= Tables::Codec::Subtitle();
	const auto attachment 	= Tables::Codec::Attachment();

	m_codecs.reserve(video.size() + audio.size() + subtitle.size() + attachment.size());
	m_by_name.reserve((video.size() + audio.size() + subtitle.size() + attachment.size()) * 2);
	m_by_type.reserve(4);

	Load(Type::Video, video);
	Load(Type::Audio, audio);
	Load(Type::Subtitle, subtitle);
	Load(Type::Attachment, attachment);
}
