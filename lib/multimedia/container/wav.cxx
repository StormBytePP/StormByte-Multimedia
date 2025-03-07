#include <multimedia/container/wav.hxx>

using namespace StormByte::Multimedia::Container;

WAV::WAV():Base(Type::WAV, "wav") {}

std::list<StormByte::Multimedia::Property::Type> WAV::CompatibleStreams() const noexcept {
	return {Property::Type::Audio};
}

bool WAV::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}