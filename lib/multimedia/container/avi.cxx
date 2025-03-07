#include <multimedia/container/avi.hxx>

using namespace StormByte::Multimedia::Container;

AVI::AVI():Base(Type::AVI, "avi") {}

std::list<StormByte::Multimedia::Property::Type> AVI::CompatibleStreams() const noexcept {
	return {Property::Type::Audio, Property::Type::Video};
}

bool AVI::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}