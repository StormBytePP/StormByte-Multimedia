#include <multimedia/stream/audio.hxx>

using namespace StormByte::Multimedia::Stream;

Audio::Audio(const Codec::Audio& codec, const Property::Duration& dur, const std::optional<Property::Language>& lang):
Stream(Property::Type::Audio, codec, lang), m_dur(dur) {}

Audio::Audio(Codec::Audio&& codec, Property::Duration&& dur, std::optional<Property::Language>&& lang) noexcept:
Stream(Property::Type::Audio, std::move(codec), lang), m_dur(std::move(dur)) {}

const StormByte::Multimedia::Property::Duration& Audio::GetDuration() const noexcept {
	return m_dur;
}

std::shared_ptr<Stream> Audio::Clone() const {
	return std::make_shared<Audio>(*this);
}

std::shared_ptr<Stream> Audio::Move() noexcept {
	return std::make_shared<Audio>(std::move(*this));
}