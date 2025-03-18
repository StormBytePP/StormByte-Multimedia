#include <StormByte/multimedia/stream/video.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Video::Clone() const {
	return MakePointer<Video>(*this);
}

PointerType Video::Move() {
	return MakePointer<Video>(std::move(*this));
}

Video::Video(std::shared_ptr<const Multimedia::Codec> codec) noexcept: Base(codec),
m_color(), m_resolution(), m_profile() {}

std::shared_ptr<const StormByte::Multimedia::Media::Property::Video::Color>& Video::Color() noexcept {
	return m_color;
}

const std::shared_ptr<const StormByte::Multimedia::Media::Property::Video::Color>& Video::Color() const noexcept {
	return m_color;
}

std::shared_ptr<const StormByte::Multimedia::Media::Property::Resolution>& Video::Resolution() noexcept {
	return m_resolution;
}

const std::shared_ptr<const StormByte::Multimedia::Media::Property::Resolution>& Video::Resolution() const noexcept {
	return m_resolution;
}

std::shared_ptr<const std::string>& Video::Profile() noexcept {
	return m_profile;
}

const std::shared_ptr<const std::string>& Video::Profile() const noexcept {
	return m_profile;
}
