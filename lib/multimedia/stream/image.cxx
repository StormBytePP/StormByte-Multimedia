#include <multimedia/stream/image.hxx>

using namespace StormByte::Multimedia::Stream;

Image::Image(const Property::Resolution& res):Stream(StormByte::Multimedia::Type::Image), m_res(res) {}

Image::Image(Property::Resolution&& res) noexcept:Stream(StormByte::Multimedia::Type::Image), m_res(std::move(res)) {}

const StormByte::Multimedia::Property::Resolution& Image::GetResolution() const noexcept {
	return m_res;
}