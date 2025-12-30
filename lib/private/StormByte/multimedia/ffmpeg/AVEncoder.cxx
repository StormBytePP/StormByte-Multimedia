#include <StormByte/multimedia/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/ffmpeg/AVEncoder.hxx>
#include <StormByte/multimedia/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>

using namespace StormByte::Multimedia;

FFmpeg::AVEncoder::AVEncoder(::AVCodecContext* ctx) noexcept
:m_ctx(ctx) {}

FFmpeg::AVEncoder::AVEncoder(AVEncoder&& other) noexcept
:m_ctx(other.m_ctx)
,m_stream_index(other.m_stream_index)
,m_bsf_pipeline(std::move(other.m_bsf_pipeline)) {
	other.m_ctx = nullptr;
	other.m_stream_index = -1;
}

FFmpeg::AVEncoder::~AVEncoder() noexcept {
	if (m_ctx) {
		avcodec_free_context(&m_ctx);
		m_ctx = nullptr;
	}
}

FFmpeg::AVEncoder& FFmpeg::AVEncoder::operator=(AVEncoder&& other) noexcept {
	if (this != &other) {
		if (m_ctx) {
			avcodec_free_context(&m_ctx);
			m_ctx = nullptr;
		}

		m_ctx          = other.m_ctx;
		m_stream_index = other.m_stream_index;
		m_bsf_pipeline = std::move(other.m_bsf_pipeline);

		other.m_ctx          = nullptr;
		other.m_stream_index = -1;
	}
	return *this;
}

// Open using codec parameters and optional BSF
FFmpeg::ExpectedAVEncoder FFmpeg::AVEncoder::Open(AVCodec* codec, const AVCodecParameters& params, const FFmpeg::AVFormatContext& fmt, int stream_index) noexcept {
	if (!codec || !params.m_par)
		return Unexpected<FFmpeg::EncoderError>("Invalid codec or parameters");

	// Allocate context
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
		return Unexpected<FFmpeg::EncoderError>("Out of memory allocating codec context");

	// Copy parameters into encoder context
	if (avcodec_parameters_to_context(ctx, params.m_par) < 0) {
		avcodec_free_context(&ctx);
		return Unexpected<FFmpeg::EncoderError>("Failed to copy codec parameters to encoder context");
	}

	// Open encoder
	if (avcodec_open2(ctx, codec, nullptr) < 0) {
		avcodec_free_context(&ctx);
		return Unexpected<FFmpeg::EncoderError>("Failed to open encoder");
	}

	// Construct encoder RAII wrapper
	AVEncoder enc(ctx);
	enc.m_stream_index = stream_index;

	// Checks for required BSF
	if (const char* name = fmt.NeedsMp4ToAnnexB(params.m_par->codec_id)) {
		auto expected_bsf = FFmpeg::AVBSF::Create(
			name,
			params,
			fmt.m_ctx->streams[stream_index]->time_base
		);
		if (expected_bsf)
			enc.m_bsf_pipeline.Add(std::move(expected_bsf.value()));
		else
			return Unexpected<EncoderError>("Failed to create BSF: {}", expected_bsf.error()->what());
	}

	return enc;
}

FFmpeg::OperationResult FFmpeg::AVEncoder::SendFrame(const AVFrame& frame) noexcept {
	int ret = avcodec_send_frame(m_ctx, frame.m_frame);

	switch (ret) {
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

FFmpeg::OperationResult FFmpeg::AVEncoder::ReceivePacket(AVPacket& pkt) noexcept {
	// First, pull packets from encoder into a temporary packet
	AVPacket tmp;
	int ret = avcodec_receive_packet(m_ctx, tmp.m_pkt);
	if (ret == AVERROR(EAGAIN))
		return OperationResult::TryAgain;
	if (ret == AVERROR_EOF)
		return OperationResult::EndOfFile;
	if (ret < 0)
		return OperationResult::Error;

	// Send encoded packet to BSF
	ret = m_bsf_pipeline.Process(tmp);
	if (ret < 0) {
		return OperationResult::Error;
	}

	pkt = std::move(tmp);

	if (ret == 0)
		return OperationResult::Success;
	if (ret == AVERROR(EAGAIN))
		return OperationResult::TryAgain;
	if (ret == AVERROR_EOF)
		return OperationResult::EndOfFile;

	return OperationResult::Error;
}

int FFmpeg::AVEncoder::StreamIndex() const noexcept {
	return m_stream_index;
}

void FFmpeg::AVEncoder::Flush() noexcept {
	avcodec_flush_buffers(m_ctx);
	m_bsf_pipeline.Flush();
}

void FFmpeg::AVEncoder::SetEof() noexcept {
	// Signal EOF to encoder
	avcodec_send_frame(m_ctx, nullptr);
	m_bsf_pipeline.SetEof();
}