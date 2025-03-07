#include <multimedia/codec/audio.hxx>

using namespace StormByte::Multimedia::Codec;

Audio::Audio(const std::string& name):Codec(name, Property::Type::Audio) {}

Audio::Audio(std::string&& name) noexcept:Codec(std::move(name), Property::Type::Audio) {}

