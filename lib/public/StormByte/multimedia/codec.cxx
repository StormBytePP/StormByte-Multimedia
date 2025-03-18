#include <StormByte/multimedia/codec.hxx>

using namespace StormByte::Multimedia;

Codec::Codec(const std::string& name, const std::string& description, const Media::Property::Flags& flags) noexcept:
m_name(name), m_description(description), m_flags(flags.Clone()) {}

Codec::Codec(std::string&& name, std::string&& description, Media::Property::Flags&& flags) noexcept:
m_name(std::move(name)), m_description(std::move(description)), m_flags(flags.Move()) {}

const std::string& Codec::Name() const noexcept {
	return m_name;
}

const std::string& Codec::Description() const noexcept {
	return m_description;
}

const std::shared_ptr<const Media::Property::Flags>& Codec::Flags() const noexcept {
	return m_flags;
}
