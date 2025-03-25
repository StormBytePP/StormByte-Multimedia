#include <StormByte/multimedia/media/property/size.hxx>
#include <StormByte/string.hxx>

using namespace StormByte::Multimedia::Media::Property;

std::string Size::ToString() const noexcept {
	return String::HumanReadable(s_bytes, String::Format::HumanReadableBytes);
}