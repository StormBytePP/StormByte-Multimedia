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
