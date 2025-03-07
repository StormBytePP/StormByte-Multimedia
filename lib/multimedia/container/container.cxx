#include <multimedia/container/matroska.hxx>
#include <multimedia/container/mp4.hxx>
#include <multimedia/container/avi.hxx>
#include <multimedia/container/webm.hxx>
#include <multimedia/container/mp3.hxx>
#include <multimedia/container/wav.hxx>
#include <multimedia/container/ogg.hxx>
#include <multimedia/container/ogv.hxx>
#include <multimedia/container/oga.hxx>
#include <multimedia/container/opus.hxx>
#include <multimedia/container/flac.hxx>
#include <multimedia/container/unknown.hxx>
#include <multimedia/exception.hxx>

using namespace StormByte::Multimedia::Container;

Container::Container(const Type& type, const std::string& extension):
m_type(type), m_extension(extension) {}

void Container::AddStream(const Stream::Stream& stream) {
	if (!this->CanAddStreams()) {
		throw CantAddStreams(m_type);
	}
	else if (!this->IsStreamCompatible(stream)) {
		throw StreamNotCompatible(m_type);
	}
	else
		m_streams.push_back(stream.Clone());
}

void Container::AddStream(Stream::Stream&& stream) {
	if (!this->CanAddStreams()) {
		throw CantAddStreams(m_type);
	}
	else if (!this->IsStreamCompatible(stream)) {
		throw StreamNotCompatible(m_type);
	}
	else
		m_streams.push_back(stream.Move());
}

size_t Container::GetStreamCount() const noexcept {
	return m_streams.size();
}

const std::string& Container::GetExtension() const noexcept {
	return m_extension;
}

Container::Iterator Container::Begin() noexcept {
	return Iterator::Begin(m_streams);
}

Container::ConstIterator Container::CBegin() const noexcept {
	return ConstIterator::Begin(m_streams);
}

Container::Iterator Container::End() noexcept {
	return Iterator::End(m_streams);
}

Container::ConstIterator Container::CEnd() const noexcept {
	return ConstIterator::End(m_streams);
}

Container::ConstIterator Container::Begin() const noexcept {
	return CBegin();
}

Container::ConstIterator Container::End() const noexcept {
	return CEnd();
}

std::shared_ptr<Container> Container::Create(const Type& type) {
	switch (type) {
		case Type::Matroska:	return std::make_shared<Matroska>();
		case Type::MP4:			return std::make_shared<MP4>();
		case Type::AVI:			return std::make_shared<AVI>();
		case Type::WebM:		return std::make_shared<WebM>();
		case Type::MP3:			return std::make_shared<MP3>();
		case Type::WAV:			return std::make_shared<WAV>();
		case Type::OGG:			return std::make_shared<OGG>();
		case Type::OGV:			return std::make_shared<OGV>();
		case Type::OGA:			return std::make_shared<OGA>();
		case Type::Opus:		return std::make_shared<Opus>();
		case Type::FLAC:		return std::make_shared<FLAC>();
		default:				return std::make_shared<Unknown>();
	}
	return nullptr;
}

std::shared_ptr<Container> Container::Create(const std::string& extension) {
	if (extension == ".mkv")
		return Create(Type::Matroska);
	else if (extension == ".mp4")
		return Create(Type::MP4);
	else if (extension == ".avi")
		return Create(Type::AVI);
	else if (extension == ".webm")
		return Create(Type::WebM);
	else if (extension == ".mp3")
		return Create(Type::MP3);
	else if (extension == ".wav")
		return Create(Type::WAV);
	else if (extension == ".ogg")
		return Create(Type::OGG);
	else if (extension == ".ogv")
		return Create(Type::OGV);
	else if (extension == ".oga")
		return Create(Type::OGA);
	else if (extension == ".opus")
		return Create(Type::Opus);
	else if (extension == ".flac")
		return Create(Type::FLAC);
	else
		return Create(Type::Unknown);
}

bool Container::CanAddStreams() const noexcept {
	return true;
}