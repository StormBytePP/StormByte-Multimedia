#include <multimedia/codec/subtitle.hxx>

using namespace StormByte::Multimedia::Codec;

Subtitle::Subtitle(const std::string& name):Base(name, Property::Type::Subtitle) {}

Subtitle::Subtitle(std::string&& name) noexcept:Base(std::move(name), Property::Type::Subtitle) {}
