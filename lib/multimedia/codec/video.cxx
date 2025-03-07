#include <multimedia/codec/video.hxx>

using namespace StormByte::Multimedia::Codec;

Video::Video(const std::string& name):Base(name, Property::Type::Video) {}

Video::Video(std::string&& name) noexcept:Base(std::move(name), Property::Type::Video) {}

