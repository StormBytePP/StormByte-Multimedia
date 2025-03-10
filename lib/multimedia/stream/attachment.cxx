#include <multimedia/stream/attachment.hxx>

using namespace StormByte::Multimedia::Stream;

Stream::PointerType Attachment::Clone() const {
	return MakePointer<Attachment>(*this);
}

Stream::PointerType Attachment::Move() {
	return MakePointer<Attachment>(std::move(*this));
}