#include <util/string.hxx>
#include <multimedia/property/size.hxx>

using namespace StormByte::Multimedia::Property;

std::string Size::ToString() const noexcept {
	return StormByte::Util::String::HumanReadableByteSize(s_bytes);
}