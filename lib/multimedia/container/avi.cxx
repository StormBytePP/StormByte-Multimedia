#include <multimedia/container/avi.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams AVI::CompatStreams {Property::Type::Audio, Property::Type::Video};
const CompatibleCodecs AVI::CompatCodecs {
    Codec::Name::H264, Codec::Name::MJPEG, Codec::Name::VP8, Codec::Name::VP9,
    Codec::Name::AAC, Codec::Name::AC3, Codec::Name::PCM
};


AVI::AVI():Base(Type::AVI, "avi") {}
