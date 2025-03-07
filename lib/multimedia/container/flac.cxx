#include <multimedia/container/flac.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	FLAC::CompatStreams {Property::Type::Audio};
const CompatibleCodecs	FLAC::CompatCodecs {};

FLAC::FLAC():Base(Type::FLAC, "flac") {}

const CompatibleStreams& FLAC::GetCompatibleStreams() const noexcept {
	return CompatStreams;
}

const CompatibleCodecs& FLAC::GetCompatibleCodecs() const noexcept {
	return CompatCodecs;
}