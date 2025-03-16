#include <StormByte/multimedia/media/container.hxx>

using namespace StormByte::Multimedia;

Media::Container::Info::Info(const Media::Container::Name& name, const std::string& name_string, const std::vector<Codec::Name>& allowedCodecs):
m_name(name), m_name_string(name_string), m_allowed_codecs(allowedCodecs) {}

Media::Container::Info::Info(Media::Container::Name&& name, std::string&& name_string, std::vector<Codec::Name>&& allowedCodecs):
m_name(name), m_name_string(name_string), m_allowed_codecs(allowedCodecs) {}


const Media::Container::Name& Media::Container::Info::Name() const noexcept {
	return m_name;
}
const std::string& Media::Container::Info::NameToString() const noexcept {
	return m_name_string;
}

const std::vector<Media::Codec::Name>& Media::Container::Info::AllowedCodecs() const noexcept {
	return m_allowed_codecs;
}
