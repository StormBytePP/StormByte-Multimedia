#include <StormByte/multimedia/engine/backend/ffmpeg/AVBSF.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVCodecParameters.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVFrame.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVPacket.hxx>

using namespace StormByte::Multimedia::Engine::Backend;

FFmpeg::AVBSF::AVBSF(AVBSFContext* ctx) noexcept
:AVPointer(ctx) {}

FFmpeg::AVBSF::~AVBSF() noexcept {
	Free();
}

FFmpeg::ExpectedAVBSF FFmpeg::AVBSF::Create(const std::string& name, const AVCodecParameters& params, AVRational time_base) noexcept {
	if (name.empty() || !params.Get())
		return Unexpected<BSFError>("Invalid BSF name or parameters");

	const AVBitStreamFilter* filter = av_bsf_get_by_name(name.c_str());
	if (!filter)
		return Unexpected<BSFError>("Bitstream filter not found");
	AVBSFContext* ctx = nullptr;
	if (av_bsf_alloc(filter, &ctx) < 0)
		return Unexpected<BSFError>("Failed to allocate BSF");

	if (avcodec_parameters_copy(ctx->par_in, params.Get()) < 0) {
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

FFmpeg::OperationResult FFmpeg::AVBSF::SendPacket(AVPacket& pkt) noexcept {
	int ret = av_bsf_send_packet(m_ptr, pkt.Get());

	if (ret == 0)                 return OperationResult::Success;
	if (ret == AVERROR(EAGAIN))   return OperationResult::TryAgain;
	if (ret == AVERROR_EOF)       return OperationResult::EndOfFile;

	return OperationResult::Error;
}

FFmpeg::OperationResult FFmpeg::AVBSF::ReceivePacket(AVPacket& pkt) noexcept {
	int ret = av_bsf_receive_packet(m_ptr, pkt.m_ptr);

	if (ret == 0)                 return OperationResult::Success;
	if (ret == AVERROR(EAGAIN))   return OperationResult::TryAgain;
	if (ret == AVERROR_EOF)       return OperationResult::EndOfFile;

	return OperationResult::Error;
}

void FFmpeg::AVBSF::Flush() noexcept {
	av_bsf_flush(m_ptr);
}

void FFmpeg::AVBSF::SetEof() noexcept {
	av_bsf_send_packet(m_ptr, nullptr);
}

void FFmpeg::AVBSF::Free() noexcept {
	if (m_ptr) {
		av_bsf_free(&m_ptr);
		m_ptr = nullptr;
	}
}

// Explicit template instantiation
template class StormByte::Multimedia::Engine::Backend::FFmpeg::AVPointer<::AVBSFContext>;