#include <StormByte/multimedia/stream/audio.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Audio::Clone() const {
	return MakePointer<Audio>(*this);
}

PointerType Audio::Move() {
	return MakePointer<Audio>(std::move(*this));
}

unsigned short& Audio::Channels() noexcept {
	return m_channels;
}

const unsigned short& Audio::Channels() const noexcept {
	return m_channels;
}

unsigned int& Audio::SampleRate() noexcept {
	return m_sample_rate;
}

const unsigned int& Audio::SampleRate() const noexcept {
	return m_sample_rate;
}

std::string& Audio::ChannelLayout() noexcept {
	return m_channel_layout;
}

const std::string& Audio::ChannelLayout() const noexcept {
	return m_channel_layout;
}

unsigned int& Audio::Bitrate() noexcept {
	return m_bitrate;
}

const unsigned int& Audio::Bitrate() const noexcept {
	return m_bitrate;
}

std::optional<std::string>& Audio::Profile() noexcept {
	return m_profile;
}

const std::optional<std::string>& Audio::Profile() const noexcept {
	return m_profile;
}

Audio::Audio(const Media::Codec::Name& codec) noexcept: Base(codec) {}