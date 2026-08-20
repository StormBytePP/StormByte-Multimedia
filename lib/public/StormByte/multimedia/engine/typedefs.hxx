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
