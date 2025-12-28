#include <StormByte/multimedia/stream.hxx>

using namespace StormByte::Multimedia;

Stream::Stream(const class Codec& codec, enum Type type) noexcept:
m_codec(codec), m_type(type), m_metadata() {}

enum Type Stream::Type() const noexcept {
	return m_type;
}

const class Metadata& Stream::Metadata() const noexcept {
	return m_metadata;
}

void Stream::Metadata(class Metadata&& metadata) noexcept {
	m_metadata = std::move(metadata);
}

std::shared_ptr<const Context::Generic> Stream::Context() const noexcept {
	return m_context;
}

void Stream::Context(Context::Generic&& context) noexcept {
	m_context = context.Move();
}

const class Codec& Stream::Codec() const noexcept {
	return m_codec;
}