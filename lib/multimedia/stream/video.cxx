#include <multimedia/stream/video.hxx>

using namespace StormByte::Multimedia::Stream;

Video::Video(const Codec::Video& codec, const Property::Resolution& res, const std::optional<Property::Language>& lang):
Stream(StormByte::Multimedia::Type::Video, codec, lang), m_res(res) {}

Video::Video(Codec::Video&& codec, Property::Resolution&& res, std::optional<Property::Language>&& lang) noexcept:
Stream(StormByte::Multimedia::Type::Video, std::move(codec), std::move(lang)), m_res(std::move(res)) {}

const StormByte::Multimedia::Property::Resolution& Video::GetResolution() const noexcept {
	return m_res;
}
