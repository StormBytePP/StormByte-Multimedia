#include <multimedia/stream/image.hxx>

using namespace StormByte::Multimedia::Stream;

Stream::PointerType Image::Clone() const {
	return MakePointer<Image>(*this);
}

Stream::PointerType Image::Move() {
	return MakePointer<Image>(std::move(*this));
}