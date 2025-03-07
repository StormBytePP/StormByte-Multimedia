#include <multimedia/container/mp4.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	MP4::CompatStreams {Property::Type::Audio, Property::Type::Video};
const CompatibleCodecs MP4::CompatCodecs {
    Codec::Name::H264, Codec::Name::HEVC, Codec::Name::VP9,
    Codec::Name::AAC, Codec::Name::AC3, Codec::Name::EAC3
};


MP4::MP4():Base(Type::MP4, "mp4") {}
