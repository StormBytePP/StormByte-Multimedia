#include <StormByte/string.hxx>
#include <StormByte/multimedia/context/property/hdr10.hxx>

using namespace StormByte::Multimedia::Context::Property;

/** Default values for HDR and color to check and use when metadata is missing in source video **/
const HDR10 HDR10::DEFAULT = { {34000, 16000}, {13250, 34500}, {7500, 3000}, {15635, 16450}, {1, 10000000} };

HDR10::HDR10() noexcept:
HDR10(DEFAULT.Red(), DEFAULT.Green(), DEFAULT.Blue(), DEFAULT.White(), DEFAULT.Luminance()) {}

HDR10::HDR10(const Point& red_point, const Point& green_point, const Point& blue_point, const Point& white_point, const Point& luminance, const std::optional<Point>& light_level) noexcept:
m_red(red_point), m_green(green_point), m_blue(blue_point), m_white(white_point),
m_luminance(luminance), m_light_level(light_level), m_hdr10plus(false) {}

HDR10::HDR10(Point&& red_point, Point&& green_point, Point&& blue_point, Point&& white_point, Point&& luminance, std::optional<Point>&& light_level) noexcept:
m_red(std::move(red_point)), m_green(std::move(green_point)), m_blue(std::move(blue_point)),
m_white(std::move(white_point)), m_luminance(std::move(luminance)), m_light_level(std::move(light_level)),
m_hdr10plus(false) {}

const Point& HDR10::Red() const noexcept {
	return m_red;
}

const Point& HDR10::Green() const noexcept {
	return m_green;
}

const Point& HDR10::Blue() const noexcept {
	return m_blue;
}

const Point& HDR10::White() const noexcept {
	return m_white;
}

const Point& HDR10::Luminance() const noexcept {
	return m_luminance;
}

const std::optional<Point>& HDR10::LightLevel() const noexcept {
	return m_light_level;
}

bool HDR10::IsHDR10Plus() const noexcept {
	return m_hdr10plus;
}

void HDR10::HDR10Plus(bool hdr10plus) noexcept {
	m_hdr10plus = hdr10plus;
}
