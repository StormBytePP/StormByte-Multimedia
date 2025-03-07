#include <multimedia/stream/subtitle.hxx>

using namespace StormByte::Multimedia::Stream;

Subtitle::Subtitle(const Codec::Subtitle& codec, const std::optional<Property::Language>& lang):
Stream(StormByte::Multimedia::Type::Subtitle, codec, lang) {}

Subtitle::Subtitle(Codec::Subtitle&& codec, Property::Language&& lang) noexcept:
Stream(StormByte::Multimedia::Type::Subtitle, std::move(codec), std::move(lang)) {}