#include <multimedia/codec/audio/base.hxx>

using namespace StormByte::Multimedia::Codec::Audio;

Base::Base(const Name& name):Codec::Base(name, Property::Type::Audio) {}

Base::Base(Name&& name) noexcept:Codec::Base(std::move(name), Property::Type::Audio) {}

