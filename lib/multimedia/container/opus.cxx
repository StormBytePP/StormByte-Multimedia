#include <multimedia/container/opus.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	Opus::CompatStreams {Property::Type::Audio};
const CompatibleCodecs	Opus::CompatCodecs {};

Opus::Opus():Base(Type::Opus, "opus") {}

const CompatibleStreams& Opus::GetCompatibleStreams() const noexcept {
	return CompatStreams;
}

const CompatibleCodecs& Opus::GetCompatibleCodecs() const noexcept {
	return CompatCodecs;
}