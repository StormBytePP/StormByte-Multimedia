#include <StormByte/multimedia/media/codec.hxx>

using namespace StormByte::Multimedia::Media;

bool Codec::operator==(const Codec& other) const noexcept {
	return this == &other;
}

bool Codec::operator!=(const Codec& other) const noexcept {
	return !(*this == other);
}

bool Codec::HasAccess(Access access) const noexcept {
	return m_access.Has(access);
}
