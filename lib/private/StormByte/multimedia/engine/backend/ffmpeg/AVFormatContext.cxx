#include <StormByte/multimedia/engine/backend/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVPacket.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVStream.hxx>

using namespace StormByte::Multimedia::Engine::Backend;

FFmpeg::AVFormatContext::AVFormatContext(::AVFormatContext* ctx) noexcept:
AVPointer(ctx) {}

FFmpeg::AVFormatContext::~AVFormatContext() noexcept {
	Free();
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

StormByte::Multimedia::Metadata FFmpeg::AVFormatContext::Metadata() const noexcept {
	StormByte::Multimedia::Metadata metadata;

	if (m_ptr->metadata) {
		AVDictionaryEntry* tag = nullptr;
		while ((tag = av_dict_get(m_ptr->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
			metadata[tag->key] = tag->value;
		}
	}

	return metadata;
}

FFmpeg::OperationResult FFmpeg::AVFormatContext::ReadPacket(AVPacket& packet) noexcept {
	packet.Unref();
	int ret = av_read_frame(m_ptr, packet.Get());
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

	if (!m_ptr || m_ptr->nb_streams == 0)
		return out;

	for (unsigned i = 0; i < m_ptr->nb_streams; ++i) {
		out.emplace(AVStream(m_ptr->streams[i]));
	}

	return out;
}

std::optional<FFmpeg::AVBSF> FFmpeg::AVFormatContext::Mp4ToAnnexB(int codec_id, int stream_index, const AVCodecParameters& params) const noexcept {
	if (!m_ptr || !m_ptr->iformat || !m_ptr->iformat->name)
		return std::nullopt;

	const std::string fmt_name = m_ptr->iformat->name;

	bool is_mp4_like =
		fmt_name.find("mp4") != std::string::npos ||
		fmt_name.find("isom") != std::string::npos ||
		fmt_name.find("mov") != std::string::npos;

	if (!is_mp4_like)
		return std::nullopt;

	std::string bsf_name;
	switch (codec_id) {
		case AV_CODEC_ID_HEVC: bsf_name = "hevc_mp4toannexb"; break;
		case AV_CODEC_ID_H264: bsf_name = "h264_mp4toannexb"; break;
		case AV_CODEC_ID_AV1:  bsf_name = "av1_mp4toannexb"; break;
		default:               return std::nullopt;
	}

	// Construct the AVBSF
	auto expected_bsf = FFmpeg::AVBSF::Create(
		bsf_name,
		params,
		m_ptr->streams[stream_index]->time_base
	);

	if (expected_bsf)
		return std::move(expected_bsf.value());
	else
		return std::nullopt;
}

void FFmpeg::AVFormatContext::Free() noexcept {
	if (m_ptr) {
		avformat_close_input(&m_ptr);
		m_ptr = nullptr;
	}
}

// Explicitly instantiate AVPointer for AVFormatContext*
template class StormByte::Multimedia::Engine::Backend::FFmpeg::AVPointer<::AVFormatContext>;