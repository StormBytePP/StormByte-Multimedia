#include <multimedia/stream/audio.hxx>

using namespace StormByte::Multimedia::Stream;

Audio::Audio(const Codec::Audio::Base& codec, const Property::Duration& dur, const std::optional<Property::Language>& lang):
Base(Property::Type::Audio, codec, lang), m_dur(dur) {}

Audio::Audio(Codec::Audio::Base&& codec, Property::Duration&& dur, std::optional<Property::Language>&& lang) noexcept:
Base(Property::Type::Audio, std::move(codec), lang), m_dur(std::move(dur)) {}

const StormByte::Multimedia::Property::Duration& Audio::GetDuration() const noexcept {
	return m_dur;
}

Audio::PointerType Audio::Clone() const {
	return MakePointer<Audio>(*this);
}

Audio::PointerType Audio::Move() noexcept {
	return MakePointer<Audio>(std::move(*this));
}