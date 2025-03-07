#include <multimedia/container/webm.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	WebM::CompatStreams {Property::Type::Audio};
const CompatibleCodecs WebM::CompatCodecs {
    Codec::Name::VP8, Codec::Name::VP9, Codec::Name::OPUS, Codec::Name::VORBIS
};


WebM::WebM():Base(Type::WebM, "webm") {}
