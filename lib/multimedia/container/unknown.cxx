#include <multimedia/container/unknown.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	Unknown::CompatStreams {Property::Type::Audio, Property::Type::Video, Property::Type::Subtitle, Property::Type::Image};
const CompatibleCodecs	Unknown::CompatCodecs {};

Unknown::Unknown():Base(Type::Unknown, "???") {}

const CompatibleStreams& Unknown::GetCompatibleStreams() const noexcept {
	return CompatStreams;
}

const CompatibleCodecs& Unknown::GetCompatibleCodecs() const noexcept {
	return CompatCodecs;
}