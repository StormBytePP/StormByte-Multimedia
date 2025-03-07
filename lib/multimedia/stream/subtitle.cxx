#include <multimedia/stream/subtitle.hxx>

using namespace StormByte::Multimedia::Stream;

Subtitle::Subtitle(const Codec::Subtitle::Base& codec, const std::optional<Property::Language>& lang):
Base(Property::Type::Subtitle, codec, lang) {}

Subtitle::Subtitle(Codec::Subtitle::Base&& codec, Property::Language&& lang) noexcept:
Base(Property::Type::Subtitle, std::move(codec), std::move(lang)) {}

std::shared_ptr<Base> Subtitle::Clone() const {
	return std::make_shared<Subtitle>(*this);
}

std::shared_ptr<Base> Subtitle::Move() noexcept {
	return std::make_shared<Subtitle>(std::move(*this));
}