#include <StormByte/multimedia/media/property/video/hdr10.hxx>

using namespace StormByte::Multimedia::Media::Property::Video;

/** Default values for HDR and color to check and use when metadata is missing in source video **/
const HDR10 HDR10::Default = { "yuv420p10le", "tv", "bt2020nc", "bt2020", "smpte2084", {34000, 16000}, {13250, 34500}, {7500, 3000}, {15635, 16450}, {1, 10000000} };

HDR10::HDR10(const std::string& pix_fmt, const std::string& range, const std::string& space, const std::string& primaries, const std::string& transfer,
			const Color::Point& red_point,
			const Color::Point& green_point,
			const Color::Point& blue_point,
			const Color::Point& white_point,
			const Color::Point& luminance,
			const std::optional<Color::Point>& light_level):
Color(pix_fmt, range, space), m_pix_fmt(pix_fmt), m_primaries(primaries), m_transfer(transfer),
m_red(red_point), m_green(green_point), m_blue(blue_point), m_white(white_point),
m_luminance(luminance), m_light_level(light_level), m_hdr10plus(false) {}

HDR10::HDR10(std::string&& pix_fmt, std::string&& range, std::string&& space, std::string&& primaries, std::string&& transfer,
			Color::Point&& red_point,
			Color::Point&& green_point,
			Color::Point&& blue_point,
			Color::Point&& white_point,
			Color::Point&& luminance,
			std::optional<Color::Point>&& light_level):
Color(std::move(pix_fmt), std::move(range), std::move(space)), m_primaries(std::move(primaries)), m_transfer(std::move(transfer)),
m_red(std::move(red_point)), m_green(std::move(green_point)), m_blue(std::move(blue_point)),
m_white(std::move(white_point)), m_luminance(std::move(luminance)), m_light_level(std::move(light_level)),
m_hdr10plus(false) {}

const Color::Point& HDR10::RedPoint() const noexcept {
	return m_red;
}

const Color::Point& HDR10::GreenPoint() const noexcept {
	return m_green;
}

const Color::Point& HDR10::BluePoint() const noexcept {
	return m_blue;
}

const Color::Point& HDR10::WhitePoint() const noexcept {
	return m_white;
}

const Color::Point& HDR10::Luminance() const noexcept {
	return m_luminance;
}

const std::optional<Color::Point>& HDR10::LightLevel() const noexcept {
	return m_light_level;
}

bool HDR10::IsHDR10Plus() const noexcept {
	return m_hdr10plus;
}

bool HDR10::HDR10Plus() noexcept {
	return m_hdr10plus;
}