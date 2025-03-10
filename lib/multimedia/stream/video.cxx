#include <multimedia/stream/video.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Video::Clone() const {
	return MakePointer<Video>(*this);
}

PointerType Video::Move() {
	return MakePointer<Video>(std::move(*this));
}