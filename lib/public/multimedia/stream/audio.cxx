#include <multimedia/stream/audio.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Audio::Clone() const {
	return MakePointer<Audio>(*this);
}

PointerType Audio::Move() {
	return MakePointer<Audio>(std::move(*this));
}