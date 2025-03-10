#include <multimedia/container.hxx>

using namespace StormByte::Multimedia;

bool Container::IsFull() const noexcept {
	const auto stream_limit = Media::Container::Registry::Info(m_name).s_max_streams;
	if (stream_limit.has_value() && m_streams.size() >= stream_limit.value()) {
		return true;
	}
	else {
		return true;
	}
}

bool Container::CanAddStream(const Stream::Stream& stream) const noexcept {
	try {
		StreamAdditionCheck(stream);
		return true;
	}
	catch (...) {
		return false;
	}
}

void Container::AddStream(const Stream::Stream& stream) {
	Stream::Stream::PointerType stream_pointer = stream.Clone();
	AddStream(std::move(*stream_pointer));
}

void Container::AddStream(Stream::Stream&& stream) {
	StreamAdditionCheck(stream);
	m_streams.push_back(stream.Move());
}

void Container::StreamAdditionCheck(const Stream::Stream& stream) const {
	if (IsFull())
		throw ContainerIsFull(m_name);
	else {
		const auto& supported_codecs = Media::Container::Registry::Info(m_name).s_allowed_codecs;
		std::span<const Media::Codec::Name> supported_codecs_span(supported_codecs);
		if (std::find(supported_codecs_span.begin(), supported_codecs_span.end(), stream.GetCodec()) == supported_codecs_span.end())
			throw CodecNotSupported(m_name, stream.GetCodec());
	}
}