#include <StormByte/multimedia/engine/stream.hxx>

using namespace StormByte::Multimedia::Engine;

Stream::Stream(const class Codec& codec, enum Type type) noexcept:
m_codec(codec), m_type(type), m_metadata() {
	// If codec is mjpeg, then stream will be set as Video when it is Attachment
	if (codec.Name() == "mjpeg") {
		m_type = Type::Attachment;
	}
}

Multimedia::Type Stream::Type() const noexcept {
	return m_type;
}

const StormByte::Multimedia::Metadata& Stream::Metadata() const noexcept {
	return m_metadata;
}

void Stream::Metadata(class Metadata&& metadata) noexcept {
	m_metadata = std::move(metadata);
}

std::shared_ptr<const StormByte::Multimedia::Context::Generic> Stream::Context() const noexcept {
	return m_context;
}

void Stream::Context(StormByte::Multimedia::Context::Generic&& context) noexcept {
	m_context = context.Move();
}

const class Codec& Stream::Codec() const noexcept {
	return m_codec;
}