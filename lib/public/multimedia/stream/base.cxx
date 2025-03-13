#include <multimedia/stream.hxx> // All includes

using namespace StormByte::Multimedia::Stream;

PointerType Base::Create(const Media::Codec::Name& codec) {
	// We need to use explicitelly new here because MakePointer gives problems with friendship
	switch (Media::Codec::Registry::Info(codec).s_type) {
		case Media::Type::Audio:
			return PointerType(new Audio(codec));
		case Media::Type::Video:
			return PointerType(new Video(codec));
		case Media::Type::Subtitle:
			return PointerType(new Subtitle(codec));
		case Media::Type::Image:
			return PointerType(new Image(codec));
		case Media::Type::Attachment:
			return PointerType(new Attachment(codec));
		default:
			return nullptr;
	}
}