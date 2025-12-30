#include <StormByte/multimedia/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/ffmpeg/AVPacket.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavcodec/bsf.h>
}

using namespace StormByte::Multimedia;

FFmpeg::AVBSF::AVBSF(AVBSFContext* ctx) noexcept
: m_ctx(ctx) {}

FFmpeg::AVBSF::AVBSF(AVBSF&& other) noexcept
: m_ctx(other.m_ctx) {
	other.m_ctx = nullptr;
}

FFmpeg::AVBSF::~AVBSF() noexcept {
	if (m_ctx) {
		av_bsf_free(&m_ctx);
		m_ctx = nullptr;
	}
}

FFmpeg::AVBSF& FFmpeg::AVBSF::operator=(AVBSF&& other) noexcept {
	if (this != &other) {
		if (m_ctx)
			av_bsf_free(&m_ctx);

		m_ctx = other.m_ctx;
		other.m_ctx = nullptr;
	}
	return *this;
}

FFmpeg::ExpectedAVBSF FFmpeg::AVBSF::Create(const char* name, const AVCodecParameters& params, AVRational time_base) noexcept {
	if (!name || !params.m_par)
		return Unexpected<BSFError>("Invalid BSF name or parameters");

	const AVBitStreamFilter* filter = av_bsf_get_by_name(name);
	if (!filter)
		return Unexpected<BSFError>("Bitstream filter not found");
	AVBSFContext* ctx = nullptr;
	if (av_bsf_alloc(filter, &ctx) < 0)
		return Unexpected<BSFError>("Failed to allocate BSF");

	if (avcodec_parameters_copy(ctx->par_in, params.m_par) < 0) {
		av_bsf_free(&ctx);
		return Unexpected<BSFError>("Failed to copy parameters to BSF");
	}

	ctx->time_base_in = time_base;

	if (av_bsf_init(ctx) < 0) {
		av_bsf_free(&ctx);
		return Unexpected<BSFError>("Failed to initialize BSF");
	}

	return AVBSF(ctx);
}

FFmpeg::OperationResult FFmpeg::AVBSF::SendPacket(const AVPacket& pkt) noexcept {
	int ret = av_bsf_send_packet(m_ctx, pkt.m_pkt);

	if (ret == 0)                 return OperationResult::Success;
	if (ret == AVERROR(EAGAIN))   return OperationResult::TryAgain;
	if (ret == AVERROR_EOF)       return OperationResult::EndOfFile;

	return OperationResult::Error;
}

FFmpeg::OperationResult FFmpeg::AVBSF::ReceivePacket(AVPacket& pkt) noexcept {
	int ret = av_bsf_receive_packet(m_ctx, pkt.m_pkt);

	if (ret == 0)                 return OperationResult::Success;
	if (ret == AVERROR(EAGAIN))   return OperationResult::TryAgain;
	if (ret == AVERROR_EOF)       return OperationResult::EndOfFile;

	return OperationResult::Error;
}

void FFmpeg::AVBSF::Flush() noexcept {
	av_bsf_flush(m_ctx);
}

void FFmpeg::AVBSF::SetEof() noexcept {
	av_bsf_send_packet(m_ctx, nullptr);
}
