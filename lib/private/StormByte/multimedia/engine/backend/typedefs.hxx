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
#include <StormByte/multimedia/engine/streams.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>
#include <tuple>

/**
 * @namespace Backend
 * @brief Internal demuxer backends.
 */
namespace StormByte::Multimedia::Engine::Backend {
	using DemuxerTuple = std::tuple<Metadata, Streams>;							///< Metadata + streams
	using ExpectedDemuxerTuple = Expected<DemuxerTuple, DemuxerException>;		///< Open result
}
