#include <multimedia/container/container.hxx>
#include <multimedia/exception.hxx>

using namespace StormByte::Multimedia::Container;

Container::Container(const Type& type):m_type(type) {}

Container::~Container() noexcept {}

void Container::AddStream(const Stream::Stream& stream) {
	if (!IsStreamCompatible(stream)) {
		throw StreamNotCompatible(m_type);
	}
	m_streams.push_back(stream.Clone());
}

void Container::AddStream(Stream::Stream&& stream) noexcept {
	m_streams.push_back(stream.Move());
}

size_t Container::GetStreamCount() const noexcept {
	return m_streams.size();
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