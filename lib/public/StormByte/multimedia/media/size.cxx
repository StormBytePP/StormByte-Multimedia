#include <StormByte/multimedia/media/size.hxx>
#include <StormByte/util/string.hxx>

using namespace StormByte::Multimedia::Media;

std::string Size::ToString() const noexcept {
	return StormByte::Util::String::HumanReadableByteSize(s_bytes);
}