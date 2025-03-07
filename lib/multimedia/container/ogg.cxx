#include <multimedia/container/ogg.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	OGG::CompatStreams {Property::Type::Audio};
const CompatibleCodecs OGG::CompatCodecs {
    Codec::Name::VORBIS, Codec::Name::OPUS
};


OGG::OGG():Base(Type::OGG, "ogg") {}
