#include <multimedia/codec/video.hxx>

using namespace StormByte::Multimedia::Codec;

Video::Video(const std::string& name):Codec(name, StormByte::Multimedia::Type::Video) {}

Video::Video(std::string&& name) noexcept:Codec(std::move(name), StormByte::Multimedia::Type::Video) {}

std::shared_ptr<Codec> Video::Clone() const {
	return std::make_shared<Video>(*this);
}

std::shared_ptr<Codec> Video::Move() noexcept {
	return std::make_shared<Video>(std::move(*this));
}