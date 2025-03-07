#include <multimedia/container/mp4.hxx>

using namespace StormByte::Multimedia::Container;

MP4::MP4():Base(Type::MP4, "mp4") {}

std::list<StormByte::Multimedia::Property::Type> MP4::CompatibleStreams() const noexcept {
	return {Property::Type::Audio, Property::Type::Video};
}

bool MP4::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}