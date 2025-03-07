#include <multimedia/container/oga.hxx>

using namespace StormByte::Multimedia::Container;

OGA::OGA():Base(Type::OGA, "oga") {}

std::list<StormByte::Multimedia::Property::Type> OGA::CompatibleStreams() const noexcept {
	return {Property::Type::Audio};
}

bool OGA::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}