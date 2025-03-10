#include <multimedia/stream/subtitle.hxx>

using namespace StormByte::Multimedia::Stream;

Stream::PointerType Subtitle::Clone() const {
	return MakePointer<Subtitle>(*this);
}

Stream::PointerType Subtitle::Move() {
	return MakePointer<Subtitle>(std::move(*this));
}