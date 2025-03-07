#include <multimedia/codec/subtitle.hxx>

using namespace StormByte::Multimedia::Codec;

Subtitle::Subtitle(const std::string& name):Codec(name, Property::Type::Subtitle) {}

Subtitle::Subtitle(std::string&& name) noexcept:Codec(std::move(name), Property::Type::Subtitle) {}
