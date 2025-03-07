#include <multimedia/container/mp3.hxx>

using namespace StormByte::Multimedia::Container;

MP3::MP3():Base(Type::MP3, "mp3") {}

std::list<StormByte::Multimedia::Property::Type> MP3::CompatibleStreams() const noexcept {
	return {Property::Type::Audio};
}

bool MP3::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}