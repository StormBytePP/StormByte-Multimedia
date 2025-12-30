#include <StormByte/multimedia/encoder.hxx>
#include <StormByte/multimedia/registry/encoder.hxx>

extern "C" {
	#include <libavcodec/avcodec.h>
}

using namespace StormByte::Multimedia;

Encoder::Encoder(int id, const std::string& name) noexcept
:m_id(id), m_name(name) {
	m_features = DetectFeatures(m_name);
}

Encoder::Encoder(int id, std::string&& name) noexcept
:m_id(id), m_name(std::move(name)) {
	m_features = DetectFeatures(m_name);
}

int Encoder::CodecID() const noexcept {
	return m_id;
}

const Features& Encoder::Features() const noexcept {
	return m_features;
}

class Features Encoder::DetectFeatures(const std::string_view& name) noexcept {
	class Features features;

	/** Find decoder in registry */
	for (const auto& entry : Registry::Encoder) {
		if (entry.Name() == name) {
			features = entry.Features();
			break;
		}
	}

	/** Find ffmpeg encoder */
	if (const AVCodec* codec = avcodec_find_encoder_by_name(name.data())) {
		/** Add ffmpeg encoder caps */
		if (codec->capabilities & AV_CODEC_CAP_FRAME_THREADS)
			features |= Feature::MultiThreaded;

		if (codec->capabilities & AV_CODEC_CAP_SLICE_THREADS)
			features |= Feature::MultiThreaded; // no tienes SliceThreading

		if (codec->capabilities & AV_CODEC_CAP_HARDWARE)
			features |= Feature::HardwareAcceleration;
	}

	return features;
}