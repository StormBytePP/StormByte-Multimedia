#include <multimedia/stream/subtitle.hxx>

using namespace StormByte::Multimedia::Stream;

Subtitle::Subtitle(const Codec::Subtitle::Base& codec, const std::optional<Property::Language>& lang):
Base(Property::Type::Subtitle, codec, lang) {}

Subtitle::Subtitle(Codec::Subtitle::Base&& codec, Property::Language&& lang) noexcept:
Base(Property::Type::Subtitle, std::move(codec), std::move(lang)) {}

Subtitle::PointerType Subtitle::Clone() const {
	return MakePointer<Subtitle>(*this);
}

Subtitle::PointerType Subtitle::Move() noexcept {
	return MakePointer<Subtitle>(std::move(*this));
}