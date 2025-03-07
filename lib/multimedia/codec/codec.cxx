#include <multimedia/codec/codec.hxx>

using namespace StormByte::Multimedia::Codec;

Codec::Codec(const std::string& name, const Property::Type& type): m_name(name), m_type(type) {}

Codec::Codec(std::string&& name, const Property::Type& type) noexcept: m_name(std::move(name)), m_type(type) {}

Codec::~Codec() noexcept {}

const std::string& Codec::GetName() const noexcept {
	return m_name;
}

const StormByte::Multimedia::Property::Type& Codec::GetType() const noexcept {
	return m_type;
}