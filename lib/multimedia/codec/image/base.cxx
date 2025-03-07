#include <multimedia/codec/image/base.hxx>

using namespace StormByte::Multimedia::Codec::Image;

Base::Base(const Name& name):Codec::Base(name, Property::Type::Image) {}

Base::Base(Name&& name) noexcept:Codec::Base(std::move(name), Property::Type::Image) {}

