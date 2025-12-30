#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>

extern "C" {
	#include <libavcodec/packet.h>
}

using namespace StormByte::Multimedia;

FFmpeg::AVPacket::AVPacket() noexcept:
AVPointer(av_packet_alloc()) {}

FFmpeg::AVPacket::~AVPacket() noexcept {
	Free();
}

FFmpeg::AVPacket FFmpeg::AVPacket::Ref() const noexcept {
	AVPacket copy; // crea un AVPacket con av_packet_alloc()
	if (m_ptr && copy.m_ptr)
		av_packet_ref(copy.m_ptr, m_ptr);
	return copy;
}

void FFmpeg::AVPacket::Unref() noexcept {
	if (m_ptr) av_packet_unref(m_ptr);
}

int FFmpeg::AVPacket::StreamIndex() const noexcept {
	return m_ptr ? m_ptr->stream_index : -1;
}

void FFmpeg::AVPacket::Free() noexcept {
	if (m_ptr) {
		av_packet_free(&m_ptr);
		m_ptr = nullptr;
	}
}

// Explicit template instantiation
template class STORMBYTE_MULTIMEDIA_ADVANCED StormByte::Multimedia::FFmpeg::AVPointer<::AVPacket*>;