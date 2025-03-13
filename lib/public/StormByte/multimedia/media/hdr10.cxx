#include <StormByte/multimedia/media/hdr10.hxx>

using namespace StormByte::Multimedia::Media;

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

