#include <multimedia/container/matroska.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	Matroska::CompatStreams {Property::Type::Audio, Property::Type::Video, Property::Type::Subtitle};
const CompatibleCodecs Matroska::CompatCodecs {
    Codec::Name::H264, Codec::Name::HEVC, Codec::Name::VP9, Codec::Name::VP8, Codec::Name::AV1,
    Codec::Name::AAC, Codec::Name::AC3, Codec::Name::FLAC, Codec::Name::OPUS, Codec::Name::VORBIS,
    Codec::Name::PCM, Codec::Name::MJPEG, Codec::Name::THEORA, Codec::Name::DTS, Codec::Name::EAC3
};


Matroska::Matroska():Base(Type::Matroska, "mkv") {}
