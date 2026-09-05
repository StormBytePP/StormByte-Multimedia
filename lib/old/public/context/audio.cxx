#include <StormByte/multimedia/context/audio.hxx>

using namespace StormByte::Multimedia::Context;

Audio::Audio(unsigned int sample_rate, unsigned short channels, unsigned int bitrate, const std::optional<std::string>& profile) noexcept:
m_sample_rate(sample_rate), m_channels(channels), m_bitrate(bitrate), m_profile(profile) {}

unsigned int Audio::SampleRate() const noexcept {
	return m_sample_rate;
}

unsigned short Audio::Channels() const noexcept {
	return m_channels;
}

unsigned int Audio::Bitrate() const noexcept {
	return m_bitrate;
}

const std::optional<std::string>& Audio::Profile() const noexcept {
	return m_profile;
}

Audio::PointerType Audio::Clone() const {
	return MakePointer<Audio>(*this);
}

Audio::PointerType Audio::Move() {
	return MakePointer<Audio>(std::move(*this));
}