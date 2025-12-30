#include <StormByte/multimedia/ffmpeg/AVCodecParameters.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia;

FFmpeg::AVCodecParameters::AVCodecParameters(::AVCodecParameters* par) noexcept
:AVPointer(avcodec_parameters_alloc()) {
	if (par)
		avcodec_parameters_copy(m_ptr, par);
}

FFmpeg::AVCodecParameters::AVCodecParameters(const AVCodecParameters& other) noexcept
:AVPointer(avcodec_parameters_alloc()) {
	if (other.m_ptr)
		avcodec_parameters_copy(m_ptr, other.m_ptr);
}

FFmpeg::AVCodecParameters::~AVCodecParameters() noexcept {
	Free();
}

FFmpeg::AVCodecParameters& FFmpeg::AVCodecParameters::operator=(const AVCodecParameters& other) noexcept {
	if (this != &other) {
		Free();
		m_ptr = avcodec_parameters_alloc();
		avcodec_parameters_copy(m_ptr, other.m_ptr);
	}
	return *this;
}

void FFmpeg::AVCodecParameters::Free() noexcept {
	if (m_ptr) {
		avcodec_parameters_free(&m_ptr);
		m_ptr = nullptr;
	}
}

// Explicit template instantiation
template class STORMBYTE_MULTIMEDIA_ADVANCED StormByte::Multimedia::FFmpeg::AVPointer<::AVCodecParameters*>;