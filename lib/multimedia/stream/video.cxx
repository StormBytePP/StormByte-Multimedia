#include <multimedia/stream/video.hxx>

using namespace StormByte::Multimedia::Stream;

Stream::PointerType Video::Clone() const {
	return MakePointer<Video>(*this);
}

Stream::PointerType Video::Move() {
	return MakePointer<Video>(std::move(*this));
}