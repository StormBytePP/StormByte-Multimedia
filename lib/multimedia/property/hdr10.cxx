#include <multimedia/property/hdr10.hxx>

using namespace StormByte::Multimedia::Property;

/** Default values for HDR and color to check and use when metadata is missing in source video **/
const HDR10 HDR10::Default = { {34000, 16000}, {13250, 34500}, {7500, 3000}, {15635, 16450}, {1, 10000000} };

HDR10::HDR10(const Color::Point& red_point,
			const Color::Point& green_point,
			const Color::Point& blue_point,
			const Color::Point& white_point,
			const Color::Point& luminance,
			const std::optional<Color::Point>& light_level):
m_red(red_point), m_green(green_point), m_blue(blue_point), m_white(white_point),
m_luminance(luminance), m_light_level(light_level), m_hdr10plus(false) {}

HDR10::HDR10(Color::Point&& red_point,
			Color::Point&& green_point,
			Color::Point&& blue_point,
			Color::Point&& white_point,
			Color::Point&& luminance,
			std::optional<Color::Point>&& light_level):
m_red(std::move(red_point)), m_green(std::move(green_point)), m_blue(std::move(blue_point)),
m_white(std::move(white_point)), m_luminance(std::move(luminance)), m_light_level(std::move(light_level)),
m_hdr10plus(false) {}

const Color::Point& HDR10::GetRedPoint() const noexcept { return m_red; }

void HDR10::SetRedPoint(const Color::Point& red_point) { m_red = red_point; }

void HDR10::SetRedPoint(Color::Point&& red_point) noexcept { m_red = std::move(red_point); }

const Color::Point& HDR10::GetGreenPoint() const noexcept { return m_green; }

void HDR10::SetGreenPoint(const Color::Point& green_point) { m_green = green_point; }

void HDR10::SetGreenPoint(Color::Point&& green_point) noexcept { m_green = std::move(green_point); }

const Color::Point& HDR10::GetBluePoint() const noexcept { return m_blue; }

void HDR10::SetBluePoint(const Color::Point& blue_point) { m_blue = blue_point; }

void HDR10::SetBluePoint(Color::Point&& blue_point) noexcept { m_blue = std::move(blue_point); }

const Color::Point& HDR10::GetWhitePoint() const noexcept { return m_white; }

void HDR10::SetWhitePoint(const Color::Point& white_point) { m_white = white_point; }

void HDR10::SetWhitePoint(Color::Point&& white_point) noexcept { m_white = std::move(white_point); }

const Color::Point& HDR10::GetLuminance() const noexcept { return m_luminance; }

void HDR10::SetLuminance(const Color::Point& luminance) { m_luminance = luminance; }

void HDR10::SetLuminance(Color::Point&& luminance) noexcept { m_luminance = std::move(luminance); }

const std::optional<Color::Point>& HDR10::GetLightLevel() const noexcept { return m_light_level; }

void HDR10::SetLightLevel(const Color::Point& light_level) { m_light_level = light_level; }

void HDR10::SetLightLevel(Color::Point&& light_level) noexcept { m_light_level = std::move(light_level); }

bool HDR10::IsHDR10Plus() const noexcept { return m_hdr10plus; }

void HDR10::SetHDR10Plus() noexcept { m_hdr10plus = true; }

