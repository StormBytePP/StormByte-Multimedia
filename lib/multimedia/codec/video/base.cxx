#include <multimedia/codec/video/base.hxx>

using namespace StormByte::Multimedia::Codec::Video;

Base::Base(const std::string& name):Codec::Base(name, Property::Type::Video) {}

Base::Base(std::string&& name) noexcept:Codec::Base(std::move(name), Property::Type::Video) {}

