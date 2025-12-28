#include <StormByte/multimedia/context/property/color.hxx>

using namespace StormByte::Multimedia::Context::Property;

Color::Color(const std::string& pix_fmt, const std::string& range, const std::string& space,
			const std::string& primaries, const std::string& transfer) noexcept:
m_pix_fmt(pix_fmt), m_range(range), m_space(space), m_primaries(primaries), m_transfer(transfer) {}

Color::Color(std::string&& pix_fmt, std::string&& range, std::string&& space) noexcept:
m_pix_fmt(std::move(pix_fmt)), m_range(std::move(range)), m_space(std::move(space)) {}

const std::string& Color::PixelFormat() const noexcept {
	return m_pix_fmt;
}

const std::string& Color::Range() const noexcept {
	return m_range;
}

const std::string& Color::Space() const noexcept {
	return m_space;
}

const std::string& Color::Transfer() const noexcept {
	return m_transfer;
}

const std::string& Color::Primaries() const noexcept {
	return m_primaries;
}

bool Color::IsHDR10Possible() const noexcept {
	return (m_pix_fmt == "yuv420p10le" || m_pix_fmt == "yuv422p10le" || m_pix_fmt == "yuv444p10le") &&
		   (m_range == "tv" || m_range == "full") &&
		   (m_space == "bt2020nc" || m_space == "bt2020c") &&
		   (m_primaries == "bt2020") &&
		   (m_transfer == "smpte2084");
}

bool Color::IsHLGPossible() const noexcept {
	return (m_pix_fmt == "yuv420p10le" || m_pix_fmt == "yuv422p10le" || m_pix_fmt == "yuv444p10le") &&
		   (m_range == "tv" || m_range == "full") &&
		   (m_space == "bt2020nc" || m_space == "bt2020c") &&
		   (m_primaries == "bt2020") &&
		   (m_transfer == "arib-std-b67");
}