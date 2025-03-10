#include <multimedia/stream/audio.hxx>

using namespace StormByte::Multimedia::Stream;

Stream::PointerType Audio::Clone() const {
	return MakePointer<Audio>(*this);
}

Stream::PointerType Audio::Move() {
	return MakePointer<Audio>(std::move(*this));
}