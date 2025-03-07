#include <multimedia/container/ogg.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	OGG::CompatStreams {Property::Type::Audio};
const CompatibleCodecs	OGG::CompatCodecs {};

OGG::OGG():Base(Type::OGG, "ogg") {}

const CompatibleStreams& OGG::GetCompatibleStreams() const noexcept {
	return CompatStreams;
}

const CompatibleCodecs& OGG::GetCompatibleCodecs() const noexcept {
	return CompatCodecs;
}