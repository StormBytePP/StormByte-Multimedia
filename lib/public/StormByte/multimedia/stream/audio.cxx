#include <StormByte/multimedia/stream/audio.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Audio::Clone() const {
	return MakePointer<Audio>(*this);
}

PointerType Audio::Move() {
	return MakePointer<Audio>(std::move(*this));
}

std::optional<StormByte::Multimedia::Media::Property::Audio::Channels>& Audio::Channels() noexcept {
	return m_channels;
}

const std::optional<StormByte::Multimedia::Media::Property::Audio::Channels>& Audio::Channels() const noexcept {
	return m_channels;
}

std::optional<StormByte::Multimedia::Media::Property::Audio::Sample>& Audio::Sample() noexcept {
	return m_sample;
}

const std::optional<StormByte::Multimedia::Media::Property::Audio::Sample>& Audio::Sample() const noexcept {
	return m_sample;
}

std::optional<unsigned int>& Audio::Bitrate() noexcept {
	return m_bitrate;
}

const std::optional<unsigned int>& Audio::Bitrate() const noexcept {
	return m_bitrate;
}

std::optional<std::string>& Audio::Profile() noexcept {
	return m_profile;
}

const std::optional<std::string>& Audio::Profile() const noexcept {
	return m_profile;
}

Audio::Audio(const Media::Codec::ID& codec) noexcept: Base(codec),
m_channels(), m_sample(), m_bitrate(), m_profile() {}