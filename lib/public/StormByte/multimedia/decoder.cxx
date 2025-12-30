#include <StormByte/multimedia/decoder.hxx>
#include <StormByte/multimedia/registry/decoder.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia;

Decoder::Decoder(int id, const std::string& name) noexcept
:m_id(id), m_name(name) {
	m_features = DetectFeatures(m_name);
}

Decoder::Decoder(int id, std::string&& name) noexcept
:m_id(id), m_name(std::move(name)) {
	m_features = DetectFeatures(m_name);
}

int Decoder::CodecID() const noexcept {
	return m_id;
}

const Features& Decoder::Features() const noexcept {
	return m_features;
}

class Features Decoder::DetectFeatures(const std::string_view& name) noexcept {
	class Features features;

	/** Find decoder in registry */
	for (const auto& entry : Registry::Decoder) {
		if (entry.Name() == name) {
			features = entry.Features();
			break;
		}
	}

	/** Find ffmpeg decoder */
	if (const AVCodec* codec = avcodec_find_decoder_by_name(name.data())) {
		/** Add ffmpeg decoder caps */
		if (codec->capabilities & AV_CODEC_CAP_FRAME_THREADS)
			features |= Feature::MultiThreaded;

		if (codec->capabilities & AV_CODEC_CAP_SLICE_THREADS)
			features |= Feature::MultiThreaded; // no tienes SliceThreading

		if (codec->capabilities & AV_CODEC_CAP_HARDWARE)
			features |= Feature::HardwareAcceleration;
	}

	return features;
}