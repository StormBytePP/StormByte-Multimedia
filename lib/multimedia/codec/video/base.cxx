#include <multimedia/codec/video/base.hxx>

using namespace StormByte::Multimedia::Codec::Video;

Base::Base(const Name& name):Codec::Base(name, Property::Type::Video) {}

Base::Base(Name&& name) noexcept:Codec::Base(std::move(name), Property::Type::Video) {}

