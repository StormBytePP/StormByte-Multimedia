#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/media/registry.hxx>

#include <ranges>

using namespace StormByte::Multimedia;

bool Container::IsFull() const noexcept {
	const auto& stream_limit = Media::Registry::ContainerInfo(m_name).MaxStreams();
	if (stream_limit.has_value() && m_streams.size() >= stream_limit.value()) {
		return true;
	}
	else {
		return false;
	}
}

bool Container::Supports(const Media::Codec::Name& codec) const noexcept {
	const auto& supported_codecs = Media::Registry::ContainerInfo(m_name).AllowedCodecs();
	std::span<const Media::Codec::Name> supported_codecs_span(supported_codecs);
	return std::ranges::find(supported_codecs_span, codec) != supported_codecs_span.end();
}

bool Container::CanAddStream(const Stream::Base& stream) const noexcept {
	try {
		StreamAdditionCheck(stream);
		return true;
	}
	catch (...) {
		return false;
	}
}

void Container::AddStream(const Stream::Base& stream) {
	Stream::PointerType stream_pointer = stream.Clone();
	AddStream(std::move(stream_pointer));
}

void Container::AddStream(Stream::Base&& stream) {
	AddStream(stream.Move());
}

void Container::AddStream(std::shared_ptr<Stream::Base> stream) {
	StreamAdditionCheck(*stream);
	m_streams.push_back(stream);
}

void Container::StreamAdditionCheck(const Stream::Base& stream) const {
	if (IsFull())
		throw ContainerIsFull(Media::Registry::ContainerInfo(m_name).Name());
	else if (!Supports(stream.Codec())) {
		throw CodecNotSupported(Media::Registry::ContainerInfo(m_name).Name(), Media::Registry::CodecInfo(stream.Codec()).Name());
	}
}