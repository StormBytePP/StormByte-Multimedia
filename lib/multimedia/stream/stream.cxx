#include <multimedia/stream/stream.hxx>

using namespace StormByte::Multimedia::Stream;

Stream::Stream(const Type& type, const std::optional<Property::Language>& lang):
m_type(type), m_lang(lang) {}

Stream::Stream(const Type& type, std::optional<Property::Language>&& lang) noexcept:
m_type(type), m_lang(std::move(lang)) {}

Stream::~Stream() {}

const StormByte::Multimedia::Type& Stream::GetType() const noexcept {
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