#include <StormByte/multimedia/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/ffmpeg/AVEncoder.hxx>
#include <StormByte/multimedia/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

using namespace StormByte::Multimedia;

FFmpeg::AVEncoder::AVEncoder(::AVCodecContext* ctx) noexcept
:AVPointer(ctx) {}

FFmpeg::AVEncoder::~AVEncoder() noexcept {
	Free();
}

// Open using codec parameters and optional BSF
FFmpeg::ExpectedAVEncoder FFmpeg::AVEncoder::Open(AVCodec* codec, const AVCodecParameters& params, const FFmpeg::AVFormatContext& fmt, int stream_index) noexcept {
	if (!codec || !params.Get())
		return Unexpected<FFmpeg::EncoderError>("Invalid codec or parameters");

	// Allocate context
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
		return Unexpected<FFmpeg::EncoderError>("Out of memory allocating codec context");

	// Copy parameters into encoder context
	if (avcodec_parameters_to_context(ctx, params.Get()) < 0) {
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
	auto bsf = fmt.Mp4ToAnnexB(params.Get()->codec_id, stream_index, params);
	if (bsf)
		enc.m_bsf_pipeline.Add(std::move(*bsf));

	return enc;
}

FFmpeg::OperationResult FFmpeg::AVEncoder::SendFrame(AVFrame& frame) noexcept {
	int ret = avcodec_send_frame(m_ptr, frame.Get());

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
	int ret = avcodec_receive_packet(m_ptr, tmp.Get());
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
	avcodec_flush_buffers(m_ptr);
	m_bsf_pipeline.Flush();
}

void FFmpeg::AVEncoder::SetEof() noexcept {
	// Signal EOF to encoder
	avcodec_send_frame(m_ptr, nullptr);
	m_bsf_pipeline.SetEof();
}

void FFmpeg::AVEncoder::Free() noexcept {
	if (m_ptr) {
		avcodec_free_context(&m_ptr);
		m_ptr = nullptr;
	}
}

// Explicit template instantiation
template class StormByte::Multimedia::FFmpeg::AVPointer<::AVCodecContext>;