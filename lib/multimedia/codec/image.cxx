#include <multimedia/codec/image.hxx>

using namespace StormByte::Multimedia::Codec;

Image::Image(const std::string& name):Base(name, Property::Type::Image) {}

Image::Image(std::string&& name) noexcept:Base(std::move(name), Property::Type::Image) {}

