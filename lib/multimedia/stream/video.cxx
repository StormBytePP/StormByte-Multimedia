#include <multimedia/stream/video.hxx>

using namespace StormByte::Multimedia::Stream;

Video::Video(const Property::Resolution& res, const std::optional<Property::Language>& lang):
Stream(StormByte::Multimedia::Type::Video, lang), m_res(res) {}

Video::Video(Property::Resolution&& res, std::optional<Property::Language>&& lang) noexcept:
Stream(StormByte::Multimedia::Type::Video, lang), m_res(std::move(res)) {}

const StormByte::Multimedia::Property::Resolution& Video::GetResolution() const noexcept {
	return m_res;
}
