#pragma once

#include <util/templates/iterator.hxx>
#include <multimedia/codec/name.hxx>
#include <multimedia/container/type.hxx>
#include <multimedia/property/type.hxx>
#include <multimedia/stream/base.hxx>

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 * @namespace Container
 * @brief The namespace for all containers
 */
namespace StormByte::Multimedia::Container {
	using Streams 			= std::vector<std::shared_ptr<Stream::Base>>;			///< Representation for a vector of streams.
	using Iterator 			= Util::Templates::Iterator<Streams>;					///< Representation for an iterator of streams.
	using ConstIterator		= Util::Templates::ConstIterator<Streams>;				///< Representation for a const iterator of streams.
	using CompatibleStreams = std::unordered_set<Property::Type>;					///< Representation for a set of compatible streams.
	using CompatibleCodecs 	= std::unordered_set<Codec::Name>;						///< Representation for a set of compatible codecs.
	using Extensions		= std::unordered_map<std::string, Container::Type>;		///< Representation for a map of extensions and types.
}