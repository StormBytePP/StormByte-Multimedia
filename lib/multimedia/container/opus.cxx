#include <multimedia/container/opus.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	Opus::CompatStreams {Property::Type::Audio};
const CompatibleCodecs	Opus::CompatCodecs {};

Opus::Opus():Base(Type::Opus, "opus") {}
