#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>

using namespace StormByte::Multimedia;

FFmpeg::AVPacket::AVPacket() noexcept:
m_pkt(av_packet_alloc()) {}

FFmpeg::AVPacket::AVPacket(AVPacket&& other) noexcept:
m_pkt(other.m_pkt) {
	other.m_pkt = nullptr;
}

FFmpeg::AVPacket::~AVPacket() noexcept {
	if (m_pkt) {
		av_packet_free(&m_pkt);
	}
}

FFmpeg::AVPacket& FFmpeg::AVPacket::operator=(AVPacket&& other) noexcept {
	if (this != &other) {
		if (m_pkt) {
			av_packet_free(&m_pkt);
		}
		m_pkt = other.m_pkt;
		other.m_pkt = nullptr;
	}
	return *this;
}

void FFmpeg::AVPacket::Unref() noexcept {
	if (m_pkt) av_packet_unref(m_pkt);
}

int FFmpeg::AVPacket::StreamIndex() const noexcept {
	return m_pkt ? m_pkt->stream_index : -1;
}