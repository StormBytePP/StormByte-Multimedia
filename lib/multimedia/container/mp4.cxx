#include <multimedia/container/mp4.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	MP4::CompatStreams {Property::Type::Audio, Property::Type::Video};
const CompatibleCodecs	MP4::CompatCodecs {};

MP4::MP4():Base(Type::MP4, "mp4") {}

const CompatibleStreams& MP4::GetCompatibleStreams() const noexcept {
	return CompatStreams;
}

const CompatibleCodecs& MP4::GetCompatibleCodecs() const noexcept {
	return CompatCodecs;
}