#include <multimedia/container/avi.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams AVI::CompatStreams {Property::Type::Audio, Property::Type::Video};
const CompatibleCodecs 	AVI::CompatCodecs {};

AVI::AVI():Base(Type::AVI, "avi") {}

const CompatibleStreams& AVI::GetCompatibleStreams() const noexcept {
	return CompatStreams;
}

const CompatibleCodecs& AVI::GetCompatibleCodecs() const noexcept {
	return CompatCodecs;
}