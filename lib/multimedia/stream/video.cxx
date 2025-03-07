#include <multimedia/stream/video.hxx>

using namespace StormByte::Multimedia::Stream;

Video::Video(const Property::Resolution& res):Stream(StormByte::Multimedia::Type::Video), m_res(res) {}

Video::Video(Property::Resolution&& res) noexcept:Stream(StormByte::Multimedia::Type::Video), m_res(std::move(res)) {}

const StormByte::Multimedia::Property::Resolution& Video::GetResolution() const noexcept {
	return m_res;
}
