#include <StormByte/multimedia/media/registry.hxx>
#include <StormByte/multimedia/stream.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Base::Create(const Media::Codec::Name& codec) {
	// We need to use explicitelly new here because MakePointer gives problems with friendship
	switch (Media::Registry::CodecInfo(codec).Type()) {
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

StormByte::Multimedia::Media::Tags& Base::Tags() noexcept {
	return m_tags;
}

const StormByte::Multimedia::Media::Tags& Base::Tags() const noexcept {
	return m_tags;
}

Base::Base(const Media::Codec::Name& codec) noexcept: m_codec(codec), m_tags() {}