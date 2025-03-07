#include <multimedia/container/flac.hxx>

using namespace StormByte::Multimedia::Container;

FLAC::FLAC():Base(Type::FLAC, "flac") {}

std::list<StormByte::Multimedia::Property::Type> FLAC::CompatibleStreams() const noexcept {
	return {Property::Type::Audio};
}

bool FLAC::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}