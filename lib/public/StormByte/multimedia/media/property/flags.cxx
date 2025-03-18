#include <StormByte/multimedia/media/property/flags.hxx>

using namespace StormByte::Multimedia::Media::Property;

Flags::Flags(const std::string& flags): m_flags(flags) {}

std::string Flags::ToString() const {
	return m_flags;
}