#include <StormByte/multimedia/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/ffmpeg/AVDecoder.hxx>
#include <StormByte/multimedia/ffmpeg/AVFormatContext.hxx>
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

using namespace StormByte::Multimedia;

FFmpeg::AVDecoder::AVDecoder(::AVCodecContext* ctx) noexcept:
AVPointer(ctx) {}

FFmpeg::AVDecoder::~AVDecoder() noexcept {
	Free();
}

FFmpeg::ExpectedAVDecoder FFmpeg::AVDecoder::Open(AVCodec* codec, const AVCodecParameters& params, const AVFormatContext& fmt, int stream_index) noexcept {
	if (!codec || !params.Get())
		return Unexpected<DecoderError>("Invalid codec or parameters");

	// Allocate context
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
		return Unexpected<DecoderError>("Out of memory allocating codec context");

	// Copy parameters
	if (avcodec_parameters_to_context(ctx, params.Get()) < 0) {
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
	auto bsf = fmt.Mp4ToAnnexB(params.Get()->codec_id, stream_index, params);
	if (bsf)
		dec.m_bsf_pipeline.Add(std::move(*bsf));

	return dec;
}

FFmpeg::OperationResult FFmpeg::AVDecoder::SendPacket(AVPacket& pkt) noexcept {
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
	ret = avcodec_send_packet(m_ptr, filtered_pkt.Get());

	if (ret == AVERROR(EAGAIN))
		return OperationResult::TryAgain;

	if (ret == AVERROR_EOF)
		return OperationResult::EndOfFile;

	if (ret < 0)
		return OperationResult::Error;

	return OperationResult::Success;
}

FFmpeg::OperationResult FFmpeg::AVDecoder::ReceiveFrame(AVFrame& frame) noexcept {
	int ret = avcodec_receive_frame(m_ptr, frame.Get());
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
    avcodec_flush_buffers(m_ptr);
    m_bsf_pipeline.Flush();
}

void FFmpeg::AVDecoder::SetEof() noexcept {
	avcodec_send_packet(m_ptr, nullptr);
	m_bsf_pipeline.SetEof();
}

void FFmpeg::AVDecoder::Free() noexcept {
	if (m_ptr) {
		avcodec_free_context(&m_ptr);
		m_ptr = nullptr;
	}
}

// Explicit template instantiation
template class STORMBYTE_MULTIMEDIA_ADVANCED StormByte::Multimedia::FFmpeg::AVPointer<::AVCodecContext>;