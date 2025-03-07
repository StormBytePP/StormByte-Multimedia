#include <multimedia/container/flac.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	FLAC::CompatStreams {Property::Type::Audio};
const CompatibleCodecs	FLAC::CompatCodecs {};

FLAC::FLAC():Base(Type::FLAC, "flac") {}
