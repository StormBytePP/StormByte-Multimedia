#include <StormByte/multimedia/ffmpeg/AVCodecParameters.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia;

FFmpeg::AVCodecParameters::AVCodecParameters() noexcept
:m_par(avcodec_parameters_alloc()) {}

FFmpeg::AVCodecParameters::AVCodecParameters(::AVCodecParameters* par) noexcept
:m_par(avcodec_parameters_alloc()) {
	if (par)
		avcodec_parameters_copy(m_par, par);
}

FFmpeg::AVCodecParameters::AVCodecParameters(const AVCodecParameters& other) noexcept
:m_par(avcodec_parameters_alloc()) {
	if (other.m_par)
		avcodec_parameters_copy(m_par, other.m_par);
}

FFmpeg::AVCodecParameters::AVCodecParameters(AVCodecParameters&& other) noexcept
:m_par(other.m_par) {
	other.m_par = nullptr;
}

FFmpeg::AVCodecParameters::~AVCodecParameters() noexcept {
	if (m_par)
		avcodec_parameters_free(&m_par);
}

FFmpeg::AVCodecParameters& FFmpeg::AVCodecParameters::operator=(const AVCodecParameters& other) noexcept {
	if (this != &other) {
		if (!m_par)
			m_par = avcodec_parameters_alloc();
		avcodec_parameters_copy(m_par, other.m_par);
	}
	return *this;
}

FFmpeg::AVCodecParameters& FFmpeg::AVCodecParameters::operator=(AVCodecParameters&& other) noexcept {
	if (this != &other) {
		if (m_par)
			avcodec_parameters_free(&m_par);
		m_par = other.m_par;
		other.m_par = nullptr;
	}
	return *this;
}
