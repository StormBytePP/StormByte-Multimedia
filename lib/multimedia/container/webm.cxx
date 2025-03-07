#include <multimedia/container/webm.hxx>

using namespace StormByte::Multimedia::Container;

WebM::WebM():Base(Type::WebM, "webm") {}

std::list<StormByte::Multimedia::Property::Type> WebM::CompatibleStreams() const noexcept {
	return {Property::Type::Audio, Property::Type::Video, Property::Type::Subtitle, Property::Type::Image};
}

bool WebM::IsCodecCompatible(const Codec::Base&) const noexcept {
	return true;
}