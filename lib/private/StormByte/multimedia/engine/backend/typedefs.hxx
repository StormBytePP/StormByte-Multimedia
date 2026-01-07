#pragma once

#include <StormByte/expected.hxx>
#include <StormByte/multimedia/engine/exception.hxx>
#include <StormByte/multimedia/engine/streams.hxx>
#include <StormByte/multimedia/metadata.hxx>

#include <filesystem>
#include <tuple>

/**
 * @namespace Backend
 * @brief The namespace for all internal Backend related classes and functions.
 */
namespace StormByte::Multimedia::Engine::Backend {
	using DemuxerTuple = std::tuple<Metadata, Streams>;								///< Tuple type for Demuxer components
	using ExpectedDemuxerTuple = Expected<DemuxerTuple, DemuxerException>;			///< Expected type for Demuxer components or Exception
}