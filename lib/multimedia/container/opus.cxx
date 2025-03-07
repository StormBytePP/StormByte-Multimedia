#include <multimedia/container/opus.hxx>

using namespace StormByte::Multimedia::Container;

Opus::Opus():Base(Type::Opus, "opus") {}

std::list<StormByte::Multimedia::Property::Type> Opus::CompatibleStreams() const noexcept {
	return {Property::Type::Audio};
}

bool Opus::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}