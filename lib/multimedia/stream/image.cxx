#include <multimedia/stream/image.hxx>

using namespace StormByte::Multimedia::Stream;

Image::Image(const Codec::Image& codec, const Property::Resolution& res):
Stream(StormByte::Multimedia::Type::Image, codec), m_res(res) {}

Image::Image(Codec::Image&& codec, Property::Resolution&& res) noexcept:
Stream(StormByte::Multimedia::Type::Image, std::move(codec)), m_res(std::move(res)) {}

const StormByte::Multimedia::Property::Resolution& Image::GetResolution() const noexcept {
	return m_res;
}