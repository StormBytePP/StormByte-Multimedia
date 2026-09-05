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

#include <StormByte/multimedia/engine/codec.hxx>
#include <StormByte/multimedia/engine/decoder.hxx>
#include <StormByte/multimedia/engine/encoder.hxx>
#include <StormByte/multimedia/registry/codec.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia::Engine;

Codec::Codec(int codec_id, const std::string& name, const std::string& description) noexcept:
m_codec_id(codec_id), m_name(name), m_description(description) {}

std::string Codec::Name() const noexcept {
	return m_name;
}

std::string Codec::Description() const noexcept {
	return m_description;
}

ExpectedCodec Codec::Find(const std::string& name) noexcept {
	void* opaque = nullptr;
	const AVCodec* c = nullptr;

	while ((c = av_codec_iterate(&opaque))) {
		if (strcmp(c->name, name.c_str()) == 0) {
			return Codec(
				c->id,
				c->name,
				c->long_name ? c->long_name : ""
			);
		}
	}

	return Unexpected<CodecNotFound>(name);
}

ExpectedCodec Codec::Find(int id) noexcept {
	AVCodecID codec_id = static_cast<AVCodecID>(id);

	void* opaque = nullptr;
	const AVCodec* c = nullptr;
	const AVCodec* fallback = nullptr;

	// Prefer decoder
	while ((c = av_codec_iterate(&opaque))) {
		if (c->id == codec_id) {
			if (!fallback)
				fallback = c;

			if (av_codec_is_decoder(c)) {
				return Codec(
					codec_id,
					c->name,
					c->long_name ? c->long_name : ""
				);
			}
		}
	}

	// Fallback: first implementation
	if (fallback) {
		return Codec(
			codec_id,
			fallback->name,
			fallback->long_name ? fallback->long_name : ""
		);
	}

	return Unexpected<CodecNotFound>(std::to_string(id));
}

ExpectedCodec Codec::Find(Type type, const std::optional<Features>& required) noexcept {
	for (const auto& entry : Registry::Codec) {
		/* Filter by type */
		if (entry.Type() != type)
			continue;

		/* Filter by features (if requested) */
		if (required.has_value()) {
			if (!entry.Features().Has(*required))
				continue;
		}

		/* Found a matching codec */
		auto expected_codec = Codec::Find(entry.ID());
		return *expected_codec;
	}

	return required.has_value() ? Unexpected(CodecNotFound(type, *required)) : Unexpected(CodecNotFound(type));
}

Decoders Codec::Decoders() const noexcept {
	void* opaque = nullptr;
	const AVCodec* c = nullptr;
	Engine::Decoders decoders;

	while ((c = av_codec_iterate(&opaque))) {
		if (c->id == m_codec_id && av_codec_is_decoder(c))
			decoders.push_back(Decoder(c->id, c->name));
	}

	return decoders;
}

Encoders Codec::Encoders() const noexcept {
	void* opaque = nullptr;
	const AVCodec* c = nullptr;
	Engine::Encoders encoders;

	while ((c = av_codec_iterate(&opaque))) {
		if (c->id == m_codec_id && av_codec_is_encoder(c))
			encoders.push_back(Encoder(c->id, c->name));
	}

	return encoders;
}