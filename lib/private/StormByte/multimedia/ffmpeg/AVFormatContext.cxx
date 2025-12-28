#include <StormByte/multimedia/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/ffmpeg/AVStream.hxx>

using namespace StormByte::Multimedia;

bool FFmpeg::StreamLess::operator()(const AVStream& a, const AVStream& b) const noexcept {
	return a.Index() < b.Index();
}

FFmpeg::AVFormatContext::AVFormatContext(::AVFormatContext* ctx) noexcept:
m_ctx(ctx) {}

FFmpeg::AVFormatContext::AVFormatContext(AVFormatContext&& other) noexcept:
m_ctx(other.m_ctx) {
	other.m_ctx = nullptr;
}

FFmpeg::AVFormatContext::~AVFormatContext() noexcept {
	if (m_ctx) {
		avformat_close_input(&m_ctx);
	}
}

FFmpeg::AVFormatContext& FFmpeg::AVFormatContext::operator=(AVFormatContext&& other) noexcept {
	if (this != &other) {
		if (m_ctx) {
			avformat_close_input(&m_ctx);
		}
		m_ctx = other.m_ctx;
		other.m_ctx = nullptr;
	}
	return *this;
}

FFmpeg::ExpectedAVFormatContext FFmpeg::AVFormatContext::Open(const std::filesystem::path& path) {
	::AVFormatContext* raw_ctx = nullptr;
	int ret;

	// Set less verbose logging
	av_log_set_level(AV_LOG_ERROR);

	// Open input file
	if ((ret = avformat_open_input(&raw_ctx, path.string().c_str(), nullptr, nullptr)) < 0)
		return Unexpected<DecoderError>("Could not open file {}: {}", path.string(), ErrorToString(ret));

	::AVFormatContext* fmt_ctx = raw_ctx;

	// Retrieve stream information (some demuxers require this to be called)
	if ((ret = avformat_find_stream_info(fmt_ctx, nullptr)) < 0) {
		avformat_close_input(&fmt_ctx);
		return Unexpected<DecoderError>("Could not find stream information: {}", ErrorToString(ret));
	}

	return AVFormatContext(fmt_ctx);
}

class Metadata FFmpeg::AVFormatContext::Metadata() const noexcept {
	class Metadata metadata;

	if (m_ctx->metadata) {
		AVDictionaryEntry* tag = nullptr;
		while ((tag = av_dict_get(m_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
			metadata[tag->key] = tag->value;
		}
	}

	return metadata;
}

FFmpeg::OperationResult FFmpeg::AVFormatContext::ReadPacket(AVPacket& packet) noexcept {
	packet.Unref();
	int ret = av_read_frame(m_ctx, packet.m_pkt);
	switch(ret) {
		case AVERROR(EAGAIN):
			return OperationResult::TryAgain;
		case AVERROR_EOF:
			return OperationResult::EndOfFile;
		case 0:
			return OperationResult::Success;
		default:
			return OperationResult::Error;
	}
}

FFmpeg::Streams FFmpeg::AVFormatContext::Streams() const noexcept {
	FFmpeg::Streams out;

	if (!m_ctx || m_ctx->nb_streams == 0)
		return out;

	for (unsigned i = 0; i < m_ctx->nb_streams; ++i) {
		out.emplace(AVStream(m_ctx->streams[i]));
	}

	return out;
}