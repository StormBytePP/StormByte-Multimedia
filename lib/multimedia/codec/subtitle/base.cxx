#include <multimedia/codec/subtitle/base.hxx>

using namespace StormByte::Multimedia::Codec::Subtitle;

Base::Base(const std::string& name):Codec::Base(name, Property::Type::Subtitle) {}

Base::Base(std::string&& name) noexcept:Codec::Base(std::move(name), Property::Type::Subtitle) {}
