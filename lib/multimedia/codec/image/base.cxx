#include <multimedia/codec/image/base.hxx>

using namespace StormByte::Multimedia::Codec::Image;

Base::Base(const std::string& name):Codec::Base(name, Property::Type::Image) {}

Base::Base(std::string&& name) noexcept:Codec::Base(std::move(name), Property::Type::Image) {}

