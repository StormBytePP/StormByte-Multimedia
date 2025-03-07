#include <multimedia/property/resolution.hxx>

using namespace StormByte::Multimedia::Property;

std::string Resolution::GetName() const noexcept {
	return std::to_string(s_width) + "x" + std::to_string(s_height);
}

std::string Resolution::GetStandardName() const noexcept {
	if (s_height > 2160) {
		return "4K+";
	}
	else if (s_height > 1080) {
		return "4K";
	}
	else if (s_height > 720) {
		return "1080p";
	}
	else if (s_height > 480) {
		return "720p";
	}
	else if (s_height > 240) {
		return "480p";
	}
	else {
		return "240p";
	}
}