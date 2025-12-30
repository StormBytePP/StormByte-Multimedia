#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>

extern "C" {
	#include <libavutil/frame.h>
}

using namespace StormByte::Multimedia;

FFmpeg::AVFrame::AVFrame() noexcept:
AVPointer(av_frame_alloc()) {}

FFmpeg::AVFrame::~AVFrame() noexcept {
	Free();
}

void FFmpeg::AVFrame::Unref() noexcept {
	av_frame_unref(m_ptr);
}

const AVFrameSideData* FFmpeg::AVFrame::SideData(int type) const noexcept {
	if (!m_ptr)
		return nullptr;
	return av_frame_get_side_data(m_ptr, static_cast<AVFrameSideDataType>(type));
}

void FFmpeg::AVFrame::Free() noexcept {
	if (m_ptr) {
		av_frame_free(&m_ptr);
		m_ptr = nullptr;
	}
}

// Explicit template instantiation
template class STORMBYTE_MULTIMEDIA_ADVANCED StormByte::Multimedia::FFmpeg::AVPointer<::AVFrame*>;