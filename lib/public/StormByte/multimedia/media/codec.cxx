#include <StormByte/multimedia/media/codec.hxx>

using namespace StormByte::Multimedia;

Media::Codec::Info::Info(const Codec::ID& name, const Media::Type& type, const std::string& name_string) noexcept:
m_name(name), m_type(type), m_name_string(name_string) {}

Media::Codec::Info::Info(Codec::ID&& name, Media::Type&& type, std::string&& name_string) noexcept:
m_name(name), m_type(type), m_name_string(name_string) {}

const Media::Codec::ID& Media::Codec::Info::ID() const noexcept {
	return m_name;
}

const std::string& Media::Codec::Info::Name() const noexcept {
	return m_name_string;
}

const Media::Type& Media::Codec::Info::Type() const noexcept {
	return m_type;
}
