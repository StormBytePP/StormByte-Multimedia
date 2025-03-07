#include <multimedia/codec/base.hxx>

using namespace StormByte::Multimedia::Codec;

Base::Base(const Name& name, const Property::Type& type): m_name(name), m_type(type) {}

Base::Base(Name&& name, const Property::Type& type) noexcept: m_name(std::move(name)), m_type(type) {}

Base::~Base() noexcept {}

const Name& Base::GetName() const noexcept {
	return m_name;
}

const StormByte::Multimedia::Property::Type& Base::GetType() const noexcept {
	return m_type;
}