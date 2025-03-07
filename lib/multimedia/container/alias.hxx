#pragma once

#include <multimedia/codec/name.hxx>
#include <multimedia/property/type.hxx>

#include <unordered_set>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	using CompatibleStreams = std::unordered_set<Property::Type>;		///< Representation for a set of compatible streams.
	using CompatibleCodecs 	= std::unordered_set<Codec::Name>;			///< Representation for a set of compatible codecs.
}