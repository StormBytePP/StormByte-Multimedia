#include <multimedia/stream/image.hxx>

using namespace StormByte::Multimedia::Stream;

Image::Image(const Codec::Image::Base& codec, const Property::Resolution& res):
Base(Property::Type::Image, codec), m_res(res) {}

Image::Image(Codec::Image::Base&& codec, Property::Resolution&& res) noexcept:
Base(Property::Type::Image, std::move(codec)), m_res(std::move(res)) {}

const StormByte::Multimedia::Property::Resolution& Image::GetResolution() const noexcept {
	return m_res;
}

std::shared_ptr<Base> Image::Clone() const {
	return std::make_shared<Image>(*this);
}

std::shared_ptr<Base> Image::Move() noexcept {
	return std::make_shared<Image>(std::move(*this));
}