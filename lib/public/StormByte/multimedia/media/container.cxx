#include <StormByte/multimedia/media/container.hxx>

using namespace StormByte::Multimedia;

Media::Container::Info::Info(const Media::Container::ID& id, const std::string& name, const std::vector<Codec::ID>& allowedCodecs):
m_id(id), m_name(name), m_allowed_codecs(allowedCodecs) {}

Media::Container::Info::Info(Media::Container::ID&& id, std::string&& name, std::vector<Codec::ID>&& allowedCodecs):
m_id(id), m_name(name), m_allowed_codecs(allowedCodecs) {}


const Media::Container::ID& Media::Container::Info::ID() const noexcept {
	return m_id;
}
const std::string& Media::Container::Info::Name() const noexcept {
	return m_name;
}

const std::vector<Media::Codec::ID>& Media::Container::Info::AllowedCodecs() const noexcept {
	return m_allowed_codecs;
}
