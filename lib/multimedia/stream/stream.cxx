#include <multimedia/stream/stream.hxx>

using namespace StormByte::Multimedia::Stream;

Stream::Stream(const Type& type):m_type(type) {}

Stream::~Stream() {}

const StormByte::Multimedia::Type& Stream::GetType() const noexcept {
	return m_type;
}