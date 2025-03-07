#include <util/string.hxx>
#include <multimedia/container/matroska.hxx>
#include <multimedia/container/mp4.hxx>
#include <multimedia/container/avi.hxx>
#include <multimedia/container/webm.hxx>
#include <multimedia/container/mp3.hxx>
#include <multimedia/container/wav.hxx>
#include <multimedia/container/ogg.hxx>
#include <multimedia/container/oga.hxx>
#include <multimedia/container/opus.hxx>
#include <multimedia/container/flac.hxx>
#include <multimedia/container/unknown.hxx>
#include <multimedia/exception.hxx>

using namespace StormByte::Multimedia::Container;

const Extensions Base::c_extensions = {
	{ ".mkv", Type::Matroska },
	{ ".mp4", Type::MP4 },
	{ ".avi", Type::AVI },
	{ ".webm", Type::WebM },
	{ ".mp3", Type::MP3 },
	{ ".wav", Type::WAV },
	{ ".ogg", Type::OGG },
	{ ".ogv", Type::OGG },
	{ ".oga", Type::OGA },
	{ ".opus", Type::Opus },
	{ ".flac", Type::FLAC }
};

Base::Base(const Type& type, const std::string& extension):
m_type(type), m_extension(Util::String::ToLower(extension)) {}

void Base::AddStream(const Stream::Base& stream) {
	CheckStreamAddition(stream);
	m_streams.push_back(stream.Clone());
}

void Base::AddStream(Stream::Base&& stream) {
	CheckStreamAddition(stream);
	m_streams.push_back(stream.Move());
}

Iterator Base::Begin() noexcept {
	return Iterator::Begin(m_streams);
}

ConstIterator Base::CBegin() const noexcept {
	return ConstIterator::Begin(m_streams);
}

Iterator Base::End() noexcept {
	return Iterator::End(m_streams);
}

ConstIterator Base::CEnd() const noexcept {
	return ConstIterator::End(m_streams);
}

ConstIterator Base::Begin() const noexcept {
	return CBegin();
}

ConstIterator Base::End() const noexcept {
	return CEnd();
}

std::shared_ptr<Base> Base::Create(const Type& type) {
	switch (type) {
		case Type::Matroska:	return std::make_shared<Matroska>();
		case Type::MP4:			return std::make_shared<MP4>();
		case Type::AVI:			return std::make_shared<AVI>();
		case Type::WebM:		return std::make_shared<WebM>();
		case Type::MP3:			return std::make_shared<MP3>();
		case Type::WAV:			return std::make_shared<WAV>();
		case Type::OGG:			return std::make_shared<OGG>();
		case Type::OGA:			return std::make_shared<OGA>();
		case Type::Opus:		return std::make_shared<Opus>();
		case Type::FLAC:		return std::make_shared<FLAC>();
		default:				return std::make_shared<Unknown>();
	}
	return nullptr;
}

std::shared_ptr<Base> Base::Create(const std::string& extension) {
	const auto it = c_extensions.find(Util::String::ToLower(extension));
	if (it != c_extensions.end()) {
		return Create(it->second);
	}
	else
		return Create(Container::Type::Unknown);
}

bool Base::CanAddStreams() const noexcept {
	return true;
}

void Base::CheckStreamAddition(const Stream::Base& stream) const {
	if (!this->CanAddStreams()) {
		throw CantAddStreams(m_type);
	}
	else if (IsStreamCompatible(stream)) {
		throw StreamNotCompatible(m_type);
	}
	else if (IsCodecCompatible(stream.GetCodec())) {
		throw CodecNotCompatible(stream.GetCodec().GetName(), TypeToString(m_type));
	}
}

bool Base::IsStreamCompatible(const Stream::Base& stream) const noexcept {
	return this->GetCompatibleStreams().contains(stream.GetType());
}

bool Base::IsCodecCompatible(const Codec::Base& codec) const noexcept {
	return this->GetCompatibleCodecs().contains(codec.GetName());
}