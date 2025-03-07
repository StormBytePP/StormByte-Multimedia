#include <multimedia/container/mp3.hxx>

using namespace StormByte::Multimedia::Container;

const CompatibleStreams	MP3::CompatStreams {Property::Type::Audio};
const CompatibleCodecs	MP3::CompatCodecs {};

MP3::MP3():Base(Type::MP3, "mp3") {}

const CompatibleStreams& MP3::GetCompatibleStreams() const noexcept {
	return CompatStreams;
}

const CompatibleCodecs& MP3::GetCompatibleCodecs() const noexcept {
	return CompatCodecs;
}