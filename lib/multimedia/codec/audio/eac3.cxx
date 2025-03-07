#include <multimedia/codec/audio/eac3.hxx>

using namespace StormByte::Multimedia::Codec::Audio;

EAC3::PointerType EAC3::Clone() const {
	return MakePointer<EAC3>(*this);
}

EAC3::PointerType EAC3::Move() noexcept {
	return MakePointer<EAC3>(std::move(*this));
}