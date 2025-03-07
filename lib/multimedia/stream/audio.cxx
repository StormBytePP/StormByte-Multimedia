#include <multimedia/stream/audio.hxx>

using namespace StormByte::Multimedia::Stream;

Audio::Audio(const Property::Duration& dur, const std::optional<Property::Language>& lang):
Stream(StormByte::Multimedia::Type::Audio, lang), m_dur(dur) {}

Audio::Audio(Property::Duration&& dur, std::optional<Property::Language>&& lang) noexcept:
Stream(StormByte::Multimedia::Type::Audio, lang), m_dur(std::move(dur)) {}

const StormByte::Multimedia::Property::Duration& Audio::GetDuration() const noexcept {
	return m_dur;
}