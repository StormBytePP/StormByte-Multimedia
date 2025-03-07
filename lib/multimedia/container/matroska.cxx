#include <multimedia/container/matroska.hxx>

using namespace StormByte::Multimedia::Container;

Matroska::Matroska():Base(Type::Matroska, "mkv") {}

std::list<StormByte::Multimedia::Property::Type> Matroska::CompatibleStreams() const noexcept {
	return {Property::Type::Audio, Property::Type::Video, Property::Type::Subtitle};
}

bool Matroska::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}