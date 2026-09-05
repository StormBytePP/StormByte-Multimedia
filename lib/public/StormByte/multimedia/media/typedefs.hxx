#pragma once

#include <StormByte/expected.hxx>
#include <StormByte/multimedia/media/exception.hxx>

#include <functional>
#include <vector>

/**
 * @namespace StormByte::Multimedia::Media
 * @brief Public media types: codecs, registry and stream kinds.
 */
namespace StormByte::Multimedia::Media {
	class Codec;

	using ExpectedCodec = StormByte::Expected<const Codec&, CodecNotFoundException>;	///< Lookup result
	using CodecRefs = std::vector<std::reference_wrapper<const Codec>>;			///< List of codec refs
}
