#include <StormByte/multimedia/context/property/resolution.hxx>

using namespace StormByte::Multimedia::Context::Property;

Resolution::Resolution(unsigned short width, unsigned short height):
m_width(width), m_height(height) {}

unsigned short Resolution::Width() const noexcept {
	return m_width;
}

unsigned short Resolution::Height() const noexcept {
	return m_height;
}

std::string Resolution::Name() const noexcept {
	return std::to_string(m_width) + "x" + std::to_string(m_height);
}

std::string Resolution::StandardName() const noexcept {
	if (m_height > 2160) {
		return "4K+";
	}
	else if (m_height > 1080) {
		return "4K";
	}
	else if (m_height > 720) {
		return "1080p";
	}
	else if (m_height > 480) {
		return "720p";
	}
	else if (m_height > 240) {
		return "480p";
	}
	else {
		return "240p";
	}
}