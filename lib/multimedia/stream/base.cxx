#include <multimedia/stream/base.hxx>

using namespace StormByte::Multimedia::Stream;

Base::Base(const Property::Type& type, const Codec::Base& codec, const std::optional<Property::Language>& lang):
m_type(type), m_codec(codec.Clone()), m_lang(lang) {}

Base::Base(const Property::Type& type, Codec::Base&& codec, std::optional<Property::Language>&& lang) noexcept:
m_type(type), m_codec(codec.Move()), m_lang(std::move(lang)) {}

Base::~Base() noexcept {}

const StormByte::Multimedia::Property::Type& Base::GetType() const noexcept {
	return m_type;
}

const std::optional<StormByte::Multimedia::Property::Language>& Base::GetLanguage() const noexcept {
	return m_lang;
}

void Base::SetLanguage(const Property::Language& lang) noexcept {
	m_lang = lang;
}

void Base::SetLanguage(Property::Language&& lang) noexcept {
	m_lang = std::move(lang);
}

std::shared_ptr<StormByte::Multimedia::Codec::Base> Base::GetCodec() const noexcept {
	return m_codec;
}