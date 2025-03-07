#include <multimedia/codec/subtitle.hxx>

using namespace StormByte::Multimedia::Codec;

Subtitle::Subtitle(const std::string& name):Codec(name, Property::Type::Subtitle) {}

Subtitle::Subtitle(std::string&& name) noexcept:Codec(std::move(name), Property::Type::Subtitle) {}

std::shared_ptr<Codec> Subtitle::Clone() const {
	return std::make_shared<Subtitle>(*this);
}

std::shared_ptr<Codec> Subtitle::Move() noexcept {
	return std::make_shared<Subtitle>(std::move(*this));
}
