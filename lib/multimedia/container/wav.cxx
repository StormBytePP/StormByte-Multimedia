#include <multimedia/container/wav.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	WAV::CompatStreams {Property::Type::Audio};
const CompatibleCodecs	WAV::CompatCodecs {};

WAV::WAV():Base(Type::WAV, "wav") {}
