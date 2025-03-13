#include <StormByte/multimedia/media/color.hxx>

using namespace StormByte::Multimedia::Media;

const Color Color::DefaultForHDR10 = { "bt2020", "bt2020nc", "smpte2084", "yuv420p10le" };

Color::Color(const std::string& primaries, const std::string& matrix, const std::string& transfer, const std::string& pix_fmt):
m_prim(primaries), m_matrix(matrix), m_transfer(transfer), m_pix_fmt(pix_fmt) {}

Color::Color(std::string&& primaries, std::string&& matrix, std::string&& transfer, std::string&& pix_fmt) noexcept:
m_prim(std::move(primaries)), m_matrix(std::move(matrix)), m_transfer(std::move(transfer)), m_pix_fmt(std::move(pix_fmt)) {}

bool Color::IsHDR10Possible() const noexcept {
	return m_prim == DefaultForHDR10.m_prim && m_matrix == DefaultForHDR10.m_matrix
	&& m_transfer == DefaultForHDR10.m_transfer && m_pix_fmt == DefaultForHDR10.m_pix_fmt;
}