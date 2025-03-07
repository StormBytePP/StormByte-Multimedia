#include <multimedia/property/color.hxx>

using namespace StormByte::Multimedia::Property;

const Color Color::DefaultForHDR10 = { "bt2020", "bt2020nc", "smpte2084", "yuv420p10le" };

Color::Color(const std::string& primaries, const std::string& matrix, const std::string& transfer, const std::string& pix_fmt):
m_prim(primaries), m_matrix(matrix), m_transfer(transfer), m_pix_fmt(pix_fmt) {}

Color::Color(std::string&& primaries, std::string&& matrix, std::string&& transfer, std::string&& pix_fmt) noexcept:
m_prim(std::move(primaries)), m_matrix(std::move(matrix)), m_transfer(std::move(transfer)), m_pix_fmt(std::move(pix_fmt)) {}

const std::optional<std::string>& Color::GetPrimaries() const noexcept { return m_prim; }

void Color::SetPrimaries(const std::string& prim) { m_prim = prim; }

void Color::SetPrimaries(std::string&& prim) noexcept { m_prim = std::move(prim); }

const std::optional<std::string>& Color::GetMatrix() const noexcept { return m_matrix; }

void Color::SetMatrix(const std::string& matrix) { m_matrix = matrix; }

void Color::SetMatrix(std::string&& matrix) noexcept { m_matrix = std::move(matrix); }

const std::optional<std::string>& Color::GetTransfer() const noexcept { return m_transfer; }

void Color::SetTransfer(const std::string& transfer) { m_transfer = transfer; }

void Color::SetTransfer(std::string&& transfer) noexcept { m_transfer = std::move(transfer); }

const std::optional<std::string>& Color::GetPixelFormat() const noexcept { return m_pix_fmt; }

void Color::SetPixelFormat(const std::string& pix_fmt) { m_pix_fmt = pix_fmt; }

void Color::SetPixelFormat(std::string&& pix_fmt) noexcept { m_pix_fmt = std::move(pix_fmt); }

bool Color::IsHDR10Possible() const noexcept {
	return m_prim == DefaultForHDR10.m_prim && m_matrix == DefaultForHDR10.m_matrix
	&& m_transfer == DefaultForHDR10.m_transfer && m_pix_fmt == DefaultForHDR10.m_pix_fmt;
}