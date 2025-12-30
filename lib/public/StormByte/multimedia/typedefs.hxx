#pragma once

#include <StormByte/expected.hxx>
#include <StormByte/multimedia/exception.hxx>

#include <list>
#include <memory>
#include <vector>

/**
 * @namespace Multimedia
 * @brief The namespace for all multimedia classes.
 */
namespace StormByte::Multimedia {
	// Forwards
	class Codec;
	class Decoder;
	class Encoder;
	class File;
	class Stream;

	using Decoders = std::list<Decoder>;									///< List of Decoders
	using Encoders = std::list<Encoder>;									///< List of Encoders
	using Streams = std::vector<Stream>;									///< Vector of Streams
	using ExpectedCodec	= StormByte::Expected<Codec, CodecNotFound>;		///< Expected type for Codec or Exception
	using ExpectedFile	= StormByte::Expected<File, FileError>;				///< Expected type for File or Exception
}