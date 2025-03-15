#include <StormByte/multimedia/media/codec.hxx>

using namespace StormByte::Multimedia;

Media::Codec::Info::Info(const std::string& name, const Media::Type& type):m_name(name), m_type(type) {}

Media::Codec::Info::Info(const std::string&& name, Media::Type&& type):m_name(name), m_type(type) {}

const std::string& Media::Codec::Info::Name() const noexcept {
	return m_name;
}

const Media::Type& Media::Codec::Info::Type() const noexcept {
	return m_type;
}
