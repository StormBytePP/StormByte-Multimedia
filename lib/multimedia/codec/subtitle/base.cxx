#include <multimedia/codec/subtitle/base.hxx>

using namespace StormByte::Multimedia::Codec::Subtitle;

Base::Base(const Name& name):Codec::Base(name, Property::Type::Subtitle) {}

Base::Base(Name&& name) noexcept:Codec::Base(std::move(name), Property::Type::Subtitle) {}
