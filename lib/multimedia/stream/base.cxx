#include <multimedia/stream/base.hxx>

using namespace StormByte::Multimedia::Stream;

Base::Base(const Property::Type& type, const Codec::Base& codec, const std::optional<Property::Language>& lang):
m_type(type), m_codec(codec.Clone()), m_lang(lang) {}

Base::Base(const Property::Type& type, Codec::Base&& codec, std::optional<Property::Language>&& lang) noexcept:
m_type(type), m_codec(codec.Move()), m_lang(std::move(lang)) {}

Base::Base(const Base& other):
m_type(other.m_type), m_codec(other.m_codec->Clone()), m_lang(other.m_lang) {}

Base& Base::operator=(const Base& other) {
	if (this != &other) {
		m_type = other.m_type;
		m_codec = other.m_codec->Clone();
		m_lang = other.m_lang;
	}
	return *this;
}

Base::~Base() noexcept {}

