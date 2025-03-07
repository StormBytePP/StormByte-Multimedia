#include <multimedia/container/oga.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	OGA::CompatStreams {Property::Type::Audio};
const CompatibleCodecs OGA::CompatCodecs {
    Codec::Name::VORBIS, Codec::Name::OPUS
};


OGA::OGA():Base(Type::OGA, "oga") {}
