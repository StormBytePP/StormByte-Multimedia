#include <StormByte/multimedia/ffmpeg/AVDecoder.hxx>
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>

extern "C" {
	#include <libavcodec/bsf.h>
}
using namespace StormByte::Multimedia;

FFmpeg::AVDecoder::AVDecoder(::AVCodecContext* ctx) noexcept:
m_ctx(ctx) {}

FFmpeg::AVDecoder::AVDecoder(AVDecoder&& other) noexcept:
	m_ctx(other.m_ctx), m_stream_index(other.m_stream_index), m_bsf(other.m_bsf) {
	other.m_ctx = nullptr;
	other.m_stream_index = -1;
	other.m_bsf = nullptr;
}

FFmpeg::AVDecoder::~AVDecoder() noexcept {
	if (m_bsf) {
		av_bsf_free(&m_bsf);
		m_bsf = nullptr;
	}
	if (m_ctx) {
		avcodec_free_context(&m_ctx);
	}
}

FFmpeg::AVDecoder& FFmpeg::AVDecoder::operator=(AVDecoder&& other) noexcept {
	if (this != &other) {
		if (m_bsf) {
			av_bsf_free(&m_bsf);
			m_bsf = nullptr;
		}
		if (m_ctx) {
			avcodec_free_context(&m_ctx);
			m_ctx = nullptr;
		}
		m_ctx = other.m_ctx;
		m_stream_index = other.m_stream_index;
		m_bsf = other.m_bsf;

		other.m_ctx = nullptr;
		other.m_stream_index = -1;
		other.m_bsf = nullptr;
	}
	return *this;
}

// Open using codec only (without bsf)
FFmpeg::ExpectedAVDecoder FFmpeg::AVDecoder::Open(AVCodec* codec) noexcept {
	if (!codec)
		return Unexpected<FFmpeg::DecoderError>("Invalid codec provided");

	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
		return Unexpected<FFmpeg::DecoderError>("Out of memory");

	if (avcodec_open2(ctx, codec, nullptr) < 0) {
		avcodec_free_context(&ctx);
		return Unexpected<FFmpeg::DecoderError>("Codec open failed");
	}

	return AVDecoder(ctx);
}

FFmpeg::ExpectedAVDecoder FFmpeg::AVDecoder::Open(AVCodec* codec, AVCodecParameters* params, int stream_index, const char* bsf_name) noexcept {
	if (!codec || !params)
		return Unexpected<DecoderError>("Invalid codec or parameters");

	// Allocate context
	AVCodecContext* ctx = avcodec_alloc_context3(codec);
	if (!ctx)
		return Unexpected<DecoderError>("Out of memory allocating codec context");

	// Copy parameters
	if (avcodec_parameters_to_context(ctx, params) < 0) {
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

	// Optional bitstream filter
	if (bsf_name) {
		const AVBitStreamFilter* filter = av_bsf_get_by_name(bsf_name);
		if (!filter)
			return Unexpected<DecoderError>("Bitstream filter not found");

		AVBSFContext* bsf = nullptr;
		if (av_bsf_alloc(filter, &bsf) < 0)
			return Unexpected<DecoderError>("Failed to allocate BSF");

		// Copy codec parameters to BSF
		if (avcodec_parameters_copy(bsf->par_in, params) < 0) {
			av_bsf_free(&bsf);
			return Unexpected<DecoderError>("Failed to copy parameters to BSF");
		}

		// Copy time base (important!)
		bsf->time_base_in = ctx->time_base;

		// Initialize BSF
		if (av_bsf_init(bsf) < 0) {
			av_bsf_free(&bsf);
			return Unexpected<DecoderError>("Failed to initialize BSF");
		}

		dec.m_bsf = bsf;
	}

	return dec;
}

FFmpeg::OperationResult FFmpeg::AVDecoder::SendPacket(const AVPacket& pkt) noexcept {
	// Ensure packet belongs to the decoder's stream
	if (pkt.StreamIndex() != m_stream_index)
		return OperationResult::Error;

	// If no BSF, send directly
	if (!m_bsf) {
		int ret = avcodec_send_packet(m_ctx, pkt.m_pkt);
		switch (ret) {
			case 0:                 return OperationResult::Success;
			case AVERROR(EAGAIN):   return OperationResult::TryAgain;
			case AVERROR_EOF:       return OperationResult::EndOfFile;
			default:                return OperationResult::Error;
		}
	}

	// --- Bitstream filter path ---

	// Clone packet into a temporary RAII packet
	AVPacket tmp;
	if (av_packet_ref(tmp.m_pkt, pkt.m_pkt) < 0)
		return OperationResult::Error;

	// Send to BSF (ownership is transferred)
	int ret = av_bsf_send_packet(m_bsf, tmp.m_pkt);
	if (ret < 0) {
		av_packet_unref(tmp.m_pkt);
		return OperationResult::Error;
	}

	// Receive filtered packets
	AVPacket out;
	while ((ret = av_bsf_receive_packet(m_bsf, out.m_pkt)) == 0) {

		int s = avcodec_send_packet(m_ctx, out.m_pkt);
		av_packet_unref(out.m_pkt);

		if (s == 0)
			continue;

		if (s == AVERROR(EAGAIN))
			return OperationResult::TryAgain;

		if (s == AVERROR_EOF)
			return OperationResult::EndOfFile;

		return OperationResult::Error;
	}

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

    if (m_bsf)
        av_bsf_flush(m_bsf);
}

void FFmpeg::AVDecoder::SetEof() noexcept {
	avcodec_send_packet(m_ctx, nullptr);

	if (m_bsf)
		av_bsf_send_packet(m_bsf, nullptr);
}