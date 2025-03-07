#include <multimedia/codec/image.hxx>

using namespace StormByte::Multimedia::Codec;

Image::Image(const std::string& name):Codec(name, Property::Type::Image) {}

Image::Image(std::string&& name) noexcept:Codec(std::move(name), Property::Type::Image) {}

