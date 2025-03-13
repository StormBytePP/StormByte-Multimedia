#include <multimedia/media/resolution.hxx>

using namespace StormByte::Multimedia::Media;

std::string Resolution::GetName() const noexcept {
	return std::to_string(m_width) + "x" + std::to_string(m_height);
}

std::string Resolution::GetStandardName() const noexcept {
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