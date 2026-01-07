#include <StormByte/multimedia/engine/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVStream.hxx>

using namespace StormByte::Multimedia::Engine::Backend;

FFmpeg::AVStream::AVStream(::AVStream* stream) noexcept
:m_stream(stream) {}

bool FFmpeg::AVStream::operator<(const AVStream& other) const noexcept {
	return Index() < other.Index();
}

int FFmpeg::AVStream::Index() const noexcept {
	return m_stream ? m_stream->index : -1;
}

int FFmpeg::AVStream::Type() const noexcept {
	return m_stream ? m_stream->codecpar->codec_type : AVMEDIA_TYPE_UNKNOWN;
}

FFmpeg::AVCodecParameters FFmpeg::AVStream::CodecParameters() const noexcept {
	return m_stream ? AVCodecParameters(m_stream->codecpar) : AVCodecParameters(nullptr);
}

AVRational FFmpeg::AVStream::TimeBase() const noexcept {
	return m_stream ? m_stream->time_base : AVRational{0, 1};
}

StormByte::Multimedia::Metadata FFmpeg::AVStream::Metadata() const noexcept {
	StormByte::Multimedia::Metadata metadata;

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