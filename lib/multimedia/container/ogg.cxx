#include <multimedia/container/ogg.hxx>

using namespace StormByte::Multimedia::Container;

OGG::OGG():Base(Type::OGG, "ogg") {}

std::list<StormByte::Multimedia::Property::Type> OGG::CompatibleStreams() const noexcept {
	return {Property::Type::Audio, Property::Type::Video, Property::Type::Subtitle};
}

bool OGG::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}