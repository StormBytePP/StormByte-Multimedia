#include <multimedia/stream/video.hxx>

using namespace StormByte::Multimedia::Stream;

Video::Video(const Codec::Video& codec, const Property::Resolution& res, const std::optional<Property::Language>& lang):
Stream(Property::Type::Video, codec, lang), m_res(res) {}

Video::Video(Codec::Video&& codec, Property::Resolution&& res, std::optional<Property::Language>&& lang) noexcept:
Stream(Property::Type::Video, std::move(codec), std::move(lang)), m_res(std::move(res)) {}

const StormByte::Multimedia::Property::Resolution& Video::GetResolution() const noexcept {
	return m_res;
}

const std::optional<StormByte::Multimedia::Property::Color>& Video::GetColor() const noexcept {
	return m_color;
}

void Video::SetColor(const StormByte::Multimedia::Property::Color& color) {
	m_color = color;
}

void Video::SetColor(StormByte::Multimedia::Property::Color&& color) noexcept {
	m_color = std::move(color);
}

const std::optional<StormByte::Multimedia::Property::HDR10>& Video::GetHDR10() const noexcept {
	return m_hdr10;
}

void Video::SetHDR10(const StormByte::Multimedia::Property::HDR10& hdr10) {
	m_hdr10 = hdr10;
}

void Video::SetHDR10(StormByte::Multimedia::Property::HDR10&& hdr10) noexcept {
	m_hdr10 = std::move(hdr10);
}