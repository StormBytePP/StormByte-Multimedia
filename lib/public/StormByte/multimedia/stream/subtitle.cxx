#include <StormByte/multimedia/stream/subtitle.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Subtitle::Clone() const {
	return MakePointer<Subtitle>(*this);
}

PointerType Subtitle::Move() {
	return MakePointer<Subtitle>(std::move(*this));
}

Subtitle::Subtitle(std::shared_ptr<const Multimedia::Codec> codec) noexcept: Base(codec) {}