#include <multimedia/container/matroska.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	Matroska::CompatStreams {Property::Type::Audio, Property::Type::Video, Property::Type::Subtitle};
const CompatibleCodecs	Matroska::CompatCodecs {};

Matroska::Matroska():Base(Type::Matroska, "mkv") {}
