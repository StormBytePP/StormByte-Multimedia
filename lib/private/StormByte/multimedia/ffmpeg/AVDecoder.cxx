#include <StormByte/multimedia/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/ffmpeg/AVDecoder.hxx>
#include <StormByte/multimedia/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>

using namespace StormByte::Multimedia;

FFmpeg::AVDecoder::AVDecoder(::AVCodecContext* ctx) noexcept:
m_ctx(ctx) {}

FFmpeg::AVDecoder::AVDecoder(AVDecoder&& other) noexcept:
	m_ctx(other.m_ctx), m_stream_index(other.m_stream_index), m_bsf_pipeline(std::move(other.m_bsf_pipeline)) {
	other.m_ctx = nullptr;
	other.m_stream_index = -1;
}

FFmpeg::AVDecoder::~AVDecoder() noexcept {
	if (m_ctx) {
		avcodec_free_context(&m_ctx);
	}
}

FFmpeg::AVDecoder& FFmpeg::AVDecoder::operator=(AVDecoder&& other) noexcept {
	if (this != &other) {
		if (m_ctx) {
			avcodec_free_context(&m_ctx);
			m_ctx = nullptr;
		}
		m_ctx = other.m_ctx;
		m_stream_index = other.m_stream_index;
		m_bsf_pipeline = std::move(other.m_bsf_pipeline);

		other.m_ctx = nullptr;
		other.m_stream_index = -1;
	}
	return *this;
}

FFmpeg::ExpectedAVDecoder FFmpeg::AVDecoder::Open(AVCodec* codec, const AVCodecParameters& params, const AVFormatContext& fmt, int stream_index) noexcept {
	if (!codec || !params.m_par)
		return Unexpected<DecoderError>("Invalid codec or parameters");

	// Allocate context
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
		return Unexpected<DecoderError>("Out of memory allocating codec context");

	// Copy parameters
	if (avcodec_parameters_to_context(ctx, params.m_par) < 0) {
		avcodec_free_context(&ctx);
		return Unexpected<DecoderError>("Failed to copy codec parameters");
	}

	// Open decoder
	if (avcodec_open2(ctx, codec, nullptr) < 0) {
		avcodec_free_context(&ctx);
		return Unexpected<DecoderError>("Failed to open decoder");
	}

	// Construct decoder RAII wrapper
	AVDecoder dec(ctx);
	dec.m_stream_index = stream_index;

	// Checks for required BSF
	if (const char* name = fmt.NeedsMp4ToAnnexB(params.m_par->codec_id)) {
		auto expected_bsf = FFmpeg::AVBSF::Create(
			name,
			params,
			fmt.m_ctx->streams[stream_index]->time_base
		);
		if (expected_bsf)
			dec.m_bsf_pipeline.Add(std::move(expected_bsf.value()));
		else
			return Unexpected<DecoderError>("Failed to create BSF: {}", expected_bsf.error()->what());
	}

	return dec;
}

FFmpeg::OperationResult FFmpeg::AVDecoder::SendPacket(const AVPacket& pkt) noexcept {
	// Ensure packet belongs to the decoder's stream
	if (pkt.StreamIndex() != m_stream_index)
		return OperationResult::Error;

	AVPacket filtered_pkt = pkt.Ref();
	// Send to BSF pipeline
	int ret = m_bsf_pipeline.Process(filtered_pkt);
	if (ret < 0) {
		return OperationResult::Error;
	}

	// Now send filtered packet to decoder
	ret = avcodec_send_packet(m_ctx, filtered_pkt.m_pkt);

	if (ret == AVERROR(EAGAIN))
		return OperationResult::TryAgain;

	if (ret == AVERROR_EOF)
		return OperationResult::EndOfFile;

	if (ret < 0)
		return OperationResult::Error;

	return OperationResult::Success;
}

FFmpeg::OperationResult FFmpeg::AVDecoder::ReceiveFrame(AVFrame& frame) noexcept {
	int ret = avcodec_receive_frame(m_ctx, frame.m_frame);
	switch(ret) {
		case 0:
			return OperationResult::Success;
		case AVERROR(EAGAIN):
			return OperationResult::TryAgain;
		case AVERROR_EOF:
			return OperationResult::EndOfFile;
		default:
			return OperationResult::Error;
	}
}

int FFmpeg::AVDecoder::StreamIndex() const noexcept {
	return m_stream_index;
}

void FFmpeg::AVDecoder::Flush() noexcept {
    avcodec_flush_buffers(m_ctx);
    m_bsf_pipeline.Flush();
}

void FFmpeg::AVDecoder::SetEof() noexcept {
	avcodec_send_packet(m_ctx, nullptr);
	m_bsf_pipeline.SetEof();
}