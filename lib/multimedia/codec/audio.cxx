#include <multimedia/codec/audio.hxx>

using namespace StormByte::Multimedia::Codec;

Audio::Audio(const std::string& name):Base(name, Property::Type::Audio) {}

Audio::Audio(std::string&& name) noexcept:Base(std::move(name), Property::Type::Audio) {}

