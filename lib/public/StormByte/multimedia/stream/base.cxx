#include <StormByte/multimedia/stream.hxx>

using namespace StormByte::Multimedia::Stream;

Base::Base(std::shared_ptr<const Multimedia::Codec> codec) noexcept: m_codec(codec), m_disposition(), m_tags() {}

PointerType Base::Create(std::shared_ptr<const Multimedia::Codec> codec) {
	// We need to use explicitelly new here because MakePointer gives problems with friendship
	switch (codec->Type()) {
		case Media::Type::Audio:
			return PointerType(new Audio(codec));
		case Media::Type::Video:
			return PointerType(new Video(codec));
		case Media::Type::Subtitle:
			return PointerType(new Subtitle(codec));
		case Media::Type::Attachment:
			return PointerType(new Attachment(codec));
		default:
			return nullptr; // This will not happen but compiler needs to be happy
	}
}

const std::shared_ptr<const StormByte::Multimedia::Codec>& Base::Codec() const noexcept {
	return m_codec;
}

StormByte::Multimedia::Media::Property::Tags<bool>& Base::Disposition() noexcept {
	return m_disposition;
}

const StormByte::Multimedia::Media::Property::Tags<bool>& Base::Disposition() const noexcept {
	return m_disposition;
}

StormByte::Multimedia::Media::Property::Tags<std::string>& Base::Tags() noexcept {
	return m_tags;
}

const StormByte::Multimedia::Media::Property::Tags<std::string>& Base::Tags() const noexcept {
	return m_tags;
}

namespace StormByte::Multimedia::Stream {
	PointerType Create(std::shared_ptr<const Multimedia::Codec> codec) {
		return Base::Create(codec);
	}
}