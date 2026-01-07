#pragma once

#include <StormByte/expected.hxx>
#include <StormByte/multimedia/engine/exception.hxx>

#include <list>

/**
 * @namespace Engine
 * @brief The namespace for all multimedia engine classes.
 */
namespace StormByte::Multimedia::Engine {
	// Forwards
	class Codec;
	class Decoder;
	class Demuxer;
	class Encoder;

	using Decoders = std::list<Decoder>;										///< List of Decoders
	using Encoders = std::list<Encoder>;										///< List of Encoders
	using ExpectedCodec	= StormByte::Expected<Codec, CodecNotFound>;			///< Expected type for Codec or Exception
	using ExpectedDemuxer = StormByte::Expected<const Demuxer, Exception>;		///< Expected type for Demuxer or Exception
}