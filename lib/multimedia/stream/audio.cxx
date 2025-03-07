#include <multimedia/stream/audio.hxx>

using namespace StormByte::Multimedia::Stream;

Audio::Audio(const Codec::Audio& codec, const Property::Duration& dur, const std::optional<Property::Language>& lang):
Stream(Property::Type::Audio, codec, lang), m_dur(dur) {}

Audio::Audio(Codec::Audio&& codec, Property::Duration&& dur, std::optional<Property::Language>&& lang) noexcept:
Stream(Property::Type::Audio, std::move(codec), lang), m_dur(std::move(dur)) {}

const StormByte::Multimedia::Property::Duration& Audio::GetDuration() const noexcept {
	return m_dur;
}