#include <multimedia/codec/video.hxx>

using namespace StormByte::Multimedia::Codec;

Video::Video(const std::string& name):Codec(name, Property::Type::Video) {}

Video::Video(std::string&& name) noexcept:Codec(std::move(name), Property::Type::Video) {}

