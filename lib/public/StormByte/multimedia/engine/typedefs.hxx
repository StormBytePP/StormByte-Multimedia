/*
 * Copyright (C) 2024-2026 David C. Manuelda (StormBytePP)
 *
 * This file is part of StormByte.
 *
 * StormByte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StormByte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with StormByte. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <StormByte/expected.hxx>
#include <StormByte/multimedia/engine/exception.hxx>

#include <list>

/**
 * @namespace Engine
 * @brief Multimedia engine (demux, codecs, backends).
 */
namespace StormByte::Multimedia::Engine {
	class Codec;
	class Decoder;
	class Demuxer;
	class Encoder;

	using Decoders = std::list<Decoder>;									///< List of decoders
	using Encoders = std::list<Encoder>;									///< List of encoders
	using ExpectedCodec = StormByte::Expected<Codec, CodecNotFound>;		///< Codec lookup result
	using ExpectedDemuxer = StormByte::Expected<const Demuxer, Exception>;	///< Demuxer open result
}
