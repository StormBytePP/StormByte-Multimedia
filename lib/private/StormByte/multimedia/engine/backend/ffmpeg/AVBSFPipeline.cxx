#include <StormByte/multimedia/engine/backend/ffmpeg/AVBSFPipeline.hxx>
#include <StormByte/multimedia/engine/backend/ffmpeg/AVPacket.hxx>

using namespace StormByte::Multimedia::Engine::Backend::FFmpeg;

AVBSFPipeline::AVBSFPipeline(AVBSFPipeline&& other) noexcept:
m_filters(std::move(other.m_filters)) {
	other.m_filters.clear();
}

AVBSFPipeline& AVBSFPipeline::operator=(AVBSFPipeline&& other) noexcept {
	if (this != &other) {
		m_filters = std::move(other.m_filters);
		other.m_filters.clear();
	}
	return *this;
}

void AVBSFPipeline::Add(AVBSF&& bsf) noexcept {
	m_filters.emplace_back(std::move(bsf));
}

OperationResult AVBSFPipeline::Process(AVPacket& pkt) noexcept {
	if (m_filters.empty())
		return OperationResult::Success;

	// Copy of the packet to process through the BSF chain
	AVPacket current = pkt.Ref();
	AVPacket next;

	for (auto& bsf : m_filters) {
		// Send the current packet to the BSF
		auto send_res = bsf.SendPacket(current);
		if (send_res != OperationResult::Success &&
			send_res != OperationResult::TryAgain)
			return send_res;

		// Receive the filtered packet into `next`
		auto recv_res = bsf.ReceivePacket(next);
		if (recv_res != OperationResult::Success)
			return recv_res;

		// The output of this filter becomes the input of the next one
		current = next.Ref();
	}

	// Finally, the final result is returned in `pkt`
	pkt = std::move(next);

	return OperationResult::Success;
}

void AVBSFPipeline::Flush() noexcept {
	for (auto& bsf : m_filters)
		bsf.Flush();
}

void AVBSFPipeline::SetEof() noexcept {
	for (auto& bsf : m_filters)
		bsf.SetEof();
}

void AVBSFPipeline::Clear() noexcept {
	m_filters.clear();
}

bool AVBSFPipeline::Empty() const noexcept {
	return m_filters.empty();
}
