#include <StormByte/multimedia/context/video.hxx>

using namespace StormByte::Multimedia::Context;

Video::Video(Property::Color&& color, Property::Resolution&& resolution, std::optional<Property::HDR10>&& hdr10) noexcept:
m_color(std::move(color)), m_resolution(std::move(resolution)) {
	if (m_color.IsHDR10Possible()) {
		if (hdr10.has_value()) {
			m_hdr10 = std::move(hdr10);
		}
		else {
			m_hdr10 = Property::HDR10::DEFAULT;
		}
	}
}

const Property::Color& Video::Color() const noexcept {
	return m_color;
}

const Property::Resolution& Video::Resolution() const noexcept {
	return m_resolution;
}

const std::optional<Property::HDR10>& Video::HDR10() const noexcept {
	return m_hdr10;
}

Video::PointerType Video::Clone() const {
	return MakePointer<Video>(*this);
}

Video::PointerType Video::Move() {
	return MakePointer<Video>(std::move(*this));
}