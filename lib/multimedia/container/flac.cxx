#include <multimedia/container/flac.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	FLAC::CompatStreams {Property::Type::Audio};
const CompatibleCodecs FLAC::CompatCodecs {
    Codec::Name::FLAC
};


FLAC::FLAC():Base(Type::FLAC, "flac") {}
