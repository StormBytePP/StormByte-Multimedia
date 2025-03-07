#include <multimedia/stream/subtitle.hxx>

using namespace StormByte::Multimedia::Stream;

Subtitle::Subtitle(const std::optional<Property::Language>& lang):
Stream(StormByte::Multimedia::Type::Subtitle, lang) {}

Subtitle::Subtitle(Property::Language&& lang) noexcept:
Stream(StormByte::Multimedia::Type::Subtitle, std::move(lang)) {}