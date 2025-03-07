#include <multimedia/container/unknown.hxx>

using namespace StormByte::Multimedia::Container;

Unknown::Unknown():Base(Type::Unknown, "???") {}

std::list<StormByte::Multimedia::Property::Type> Unknown::CompatibleStreams() const noexcept {
	return {Property::Type::Audio, Property::Type::Video, Property::Type::Subtitle, Property::Type::Image};
}

bool Unknown::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}