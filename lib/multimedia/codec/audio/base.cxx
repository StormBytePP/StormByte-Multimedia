#include <multimedia/codec/audio/base.hxx>

using namespace StormByte::Multimedia::Codec::Audio;

Base::Base(const std::string& name):Codec::Base(name, Property::Type::Audio) {}

Base::Base(std::string&& name) noexcept:Codec::Base(std::move(name), Property::Type::Audio) {}

