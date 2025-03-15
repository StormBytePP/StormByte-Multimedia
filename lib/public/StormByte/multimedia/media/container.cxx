#include <StormByte/multimedia/media/container.hxx>

using namespace StormByte::Multimedia;

Media::Container::Info::Info(const std::string& name, const std::vector<Codec::Name>& allowedCodecs, const std::optional<unsigned short>& maxStreams):
m_name(name), m_allowed_codecs(allowedCodecs), m_max_streams(maxStreams) {}

Media::Container::Info::Info(const std::string&& name, std::vector<Codec::Name>&& allowedCodecs, std::optional<unsigned short>&& maxStreams):
m_name(name), m_allowed_codecs(allowedCodecs), m_max_streams(maxStreams) {}

const std::string& Media::Container::Info::Name() const noexcept {
	return m_name;
}

const std::vector<Media::Codec::Name>& Media::Container::Info::AllowedCodecs() const noexcept {
	return m_allowed_codecs;
}

const std::optional<unsigned short>& Media::Container::Info::MaxStreams() const noexcept {
	return m_max_streams;
}