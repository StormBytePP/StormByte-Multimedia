#include <StormByte/multimedia/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/ffmpeg/AVStream.hxx>

using namespace StormByte::Multimedia;

FFmpeg::AVStream::AVStream(::AVStream* stream) noexcept
:m_stream(stream) {}

FFmpeg::AVStream::AVStream(AVStream&& other) noexcept
:m_stream(other.m_stream) {
	other.m_stream = nullptr;
}

FFmpeg::AVStream& FFmpeg::AVStream::operator=(AVStream&& other) noexcept {
	if (this != &other) {
		m_stream = other.m_stream;
		other.m_stream = nullptr;
	}
	return *this;
}

int FFmpeg::AVStream::Index() const noexcept {
	return m_stream ? m_stream->index : -1;
}

AVMediaType FFmpeg::AVStream::Type() const noexcept {
	return m_stream ? m_stream->codecpar->codec_type : AVMEDIA_TYPE_UNKNOWN;
}

FFmpeg::AVCodecParameters FFmpeg::AVStream::CodecParameters() const noexcept {
	return m_stream ? AVCodecParameters(m_stream->codecpar) : AVCodecParameters(nullptr);
}

AVRational FFmpeg::AVStream::TimeBase() const noexcept {
	return m_stream ? m_stream->time_base : AVRational{0, 1};
}

class Metadata FFmpeg::AVStream::Metadata() const noexcept {
	class Metadata metadata;

	if (m_stream && m_stream->metadata) {
		AVDictionaryEntry* tag = nullptr;
		while ((tag = av_dict_get(m_stream->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
			metadata[tag->key] = tag->value;
		}
	}

	return metadata;
}

double FFmpeg::AVStream::FrameRate() const noexcept {
	if (!m_stream)
		return 0.0;
	if (m_stream->avg_frame_rate.num && m_stream->avg_frame_rate.den)
		return av_q2d(m_stream->avg_frame_rate);
	if (m_stream->r_frame_rate.num && m_stream->r_frame_rate.den)
		return av_q2d(m_stream->r_frame_rate);
	return 0.0;
}