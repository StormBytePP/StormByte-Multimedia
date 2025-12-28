#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>

using namespace StormByte::Multimedia;

FFmpeg::AVFrame::AVFrame() noexcept:
m_frame(av_frame_alloc()) {}

FFmpeg::AVFrame::AVFrame(AVFrame&& other) noexcept:
m_frame(other.m_frame) {
	other.m_frame = nullptr;
}

FFmpeg::AVFrame::~AVFrame() noexcept {
	if (m_frame) {
		av_frame_free(&m_frame);
	}
}

FFmpeg::AVFrame& FFmpeg::AVFrame::operator=(AVFrame&& other) noexcept {
	if (this != &other) {
		if (m_frame) {
			av_frame_free(&m_frame);
		}
		m_frame = other.m_frame;
		other.m_frame = nullptr;
	}
	return *this;
}

void FFmpeg::AVFrame::Unref() noexcept {
	av_frame_unref(m_frame);
}

const AVFrameSideData* FFmpeg::AVFrame::SideData(enum AVFrameSideDataType type) const noexcept {
	if (!m_frame)
		return nullptr;
	return av_frame_get_side_data(m_frame, type);
}