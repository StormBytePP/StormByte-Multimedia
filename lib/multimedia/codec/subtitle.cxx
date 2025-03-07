#include <multimedia/codec/subtitle.hxx>

using namespace StormByte::Multimedia::Codec;

Subtitle::Subtitle(const std::string& name):Codec(name, StormByte::Multimedia::Type::Subtitle) {}

Subtitle::Subtitle(std::string&& name) noexcept:Codec(std::move(name), StormByte::Multimedia::Type::Subtitle) {}
