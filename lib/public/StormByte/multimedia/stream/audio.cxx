#include <StormByte/multimedia/stream/audio.hxx>

using namespace StormByte::Multimedia::Stream;

PointerType Audio::Clone() const {
	return MakePointer<Audio>(*this);
}

PointerType Audio::Move() {
	return MakePointer<Audio>(std::move(*this));
}

std::shared_ptr<const StormByte::Multimedia::Media::Property::Audio::Channels>& Audio::Channels() noexcept {
	return m_channels;
}

const std::shared_ptr<const StormByte::Multimedia::Media::Property::Audio::Channels>& Audio::Channels() const noexcept {
	return m_channels;
}

std::shared_ptr<const StormByte::Multimedia::Media::Property::Audio::Sample>& Audio::Sample() noexcept {
	return m_sample;
}

const std::shared_ptr<const StormByte::Multimedia::Media::Property::Audio::Sample>& Audio::Sample() const noexcept {
	return m_sample;
}

std::shared_ptr<const unsigned int>& Audio::Bitrate() noexcept {
	return m_bitrate;
}

const std::shared_ptr<const unsigned int>& Audio::Bitrate() const noexcept {
	return m_bitrate;
}

std::shared_ptr<const std::string>& Audio::Profile() noexcept {
	return m_profile;
}

const std::shared_ptr<const std::string>& Audio::Profile() const noexcept {
	return m_profile;
}

Audio::Audio(std::shared_ptr<const Multimedia::Codec> codec) noexcept: Base(codec),
m_channels(), m_sample(), m_bitrate(), m_profile() {}