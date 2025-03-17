#include <StormByte/multimedia/media/property/size.hxx>
#include <StormByte/util/string.hxx>

using namespace StormByte::Multimedia::Media::Property;

std::string Size::ToString() const noexcept {
	return StormByte::Util::String::HumanReadableByteSize(s_bytes);
}