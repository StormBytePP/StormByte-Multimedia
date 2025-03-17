#include <StormByte/multimedia/media/property/video/color.hxx>

using namespace StormByte::Multimedia::Media::Property::Video;

Color::Color(const std::string& pix_fmt, const std::string& range, const std::string& space) noexcept:
m_pix_fmt(pix_fmt), m_range(range), m_space(space) {}

Color::Color(std::string&& pix_fmt, std::string&& range, std::string&& space) noexcept:
m_pix_fmt(std::move(pix_fmt)), m_range(std::move(range)), m_space(std::move(space)) {}

const std::string& Color::PixelFormat() const noexcept {
	return m_pix_fmt;
}

const std::string& Color::Range() const noexcept {
	return m_range;
}

const std::string& Color::Space() const noexcept {
	return m_space;
}
