#include <multimedia/stream/image.hxx>

using namespace StormByte::Multimedia::Stream;

Image::Image(const Codec::Image::Base& codec, const Property::Resolution& res):
Base(Property::Type::Image, codec), m_res(res) {}

Image::Image(Codec::Image::Base&& codec, Property::Resolution&& res) noexcept:
Base(Property::Type::Image, std::move(codec)), m_res(std::move(res)) {}

const StormByte::Multimedia::Property::Resolution& Image::GetResolution() const noexcept {
	return m_res;
}

Image::PointerType Image::Clone() const {
	return MakePointer<Image>(*this);
}

Image::PointerType Image::Move() noexcept {
	return MakePointer<Image>(std::move(*this));
}