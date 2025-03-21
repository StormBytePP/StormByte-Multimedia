#include <StormByte/multimedia/stream/attachment.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Attachment::Clone() const {
	return MakePointer<Attachment>(*this);
}

PointerType Attachment::Move() {
	return MakePointer<Attachment>(std::move(*this));
}

Attachment::Attachment(std::shared_ptr<const Multimedia::Codec> codec) noexcept: Base(codec) {}