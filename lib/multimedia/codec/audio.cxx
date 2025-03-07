#include <multimedia/codec/audio.hxx>

using namespace StormByte::Multimedia::Codec;

Audio::Audio(const std::string& name):Codec(name, StormByte::Multimedia::Type::Audio) {}

Audio::Audio(std::string&& name) noexcept:Codec(std::move(name), StormByte::Multimedia::Type::Audio) {}
