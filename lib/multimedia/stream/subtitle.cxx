#include <multimedia/stream/subtitle.hxx>

using namespace StormByte::Multimedia::Stream;

Subtitle::Subtitle(const Codec::Subtitle& codec, const std::optional<Property::Language>& lang):
Stream(Property::Type::Subtitle, codec, lang) {}

Subtitle::Subtitle(Codec::Subtitle&& codec, Property::Language&& lang) noexcept:
Stream(Property::Type::Subtitle, std::move(codec), std::move(lang)) {}

std::shared_ptr<Stream> Subtitle::Clone() const {
	return std::make_shared<Subtitle>(*this);
}

std::shared_ptr<Stream> Subtitle::Move() noexcept {
	return std::make_shared<Subtitle>(std::move(*this));
}