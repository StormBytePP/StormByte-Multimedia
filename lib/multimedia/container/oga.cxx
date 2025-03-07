#include <multimedia/container/oga.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	OGA::CompatStreams {Property::Type::Audio};
const CompatibleCodecs	OGA::CompatCodecs {};

OGA::OGA():Base(Type::OGA, "oga") {}

const CompatibleStreams& OGA::GetCompatibleStreams() const noexcept {
	return CompatStreams;
}

const CompatibleCodecs& OGA::GetCompatibleCodecs() const noexcept {
	return CompatCodecs;
}