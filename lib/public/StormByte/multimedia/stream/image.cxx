#include <StormByte/multimedia/stream/image.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Image::Clone() const {
	return MakePointer<Image>(*this);
}

PointerType Image::Move() {
	return MakePointer<Image>(std::move(*this));
}

Image::Image(std::shared_ptr<Multimedia::Codec> codec) noexcept: Base(codec) {}