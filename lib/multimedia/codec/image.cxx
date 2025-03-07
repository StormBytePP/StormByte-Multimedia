#include <multimedia/codec/image.hxx>

using namespace StormByte::Multimedia::Codec;

Image::Image(const std::string& name):Codec(name, Property::Type::Image) {}

Image::Image(std::string&& name) noexcept:Codec(std::move(name), Property::Type::Image) {}

std::shared_ptr<Codec> Image::Clone() const {
	return std::make_shared<Image>(*this);
}

std::shared_ptr<Codec> Image::Move() noexcept {
	return std::make_shared<Image>(std::move(*this));
}