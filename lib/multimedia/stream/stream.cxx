#include <multimedia/stream/stream.hxx>

using namespace StormByte::Multimedia::Stream;

Stream::Stream(const Property::Type& type, const Codec::Codec& codec, const std::optional<Property::Language>& lang):
m_type(type), m_codec(codec.Clone()), m_lang(lang) {}

Stream::Stream(const Property::Type& type, Codec::Codec&& codec, std::optional<Property::Language>&& lang) noexcept:
m_type(type), m_codec(std::move(codec.Move())), m_lang(std::move(lang)) {}

Stream::~Stream() {}

const StormByte::Multimedia::Property::Type& Stream::GetType() const noexcept {
	return m_type;
}

const std::optional<StormByte::Multimedia::Property::Language>& Stream::GetLanguage() const noexcept {
	return m_lang;
}

void Stream::SetLanguage(const Property::Language& lang) noexcept {
	m_lang = lang;
}

void Stream::SetLanguage(Property::Language&& lang) noexcept {
	m_lang = std::move(lang);
}

std::shared_ptr<StormByte::Multimedia::Codec::Codec> Stream::GetCodec() const noexcept {
	return m_codec;
}