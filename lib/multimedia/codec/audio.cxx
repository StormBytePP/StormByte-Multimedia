#include <multimedia/codec/audio.hxx>

using namespace StormByte::Multimedia::Codec;

Audio::Audio(const std::string& name):Codec(name, Property::Type::Audio) {}

Audio::Audio(std::string&& name) noexcept:Codec(std::move(name), Property::Type::Audio) {}

std::shared_ptr<Codec> Audio::Clone() const {
	return std::make_shared<Audio>(*this);
}

std::shared_ptr<Codec> Audio::Move() noexcept {
	return std::make_shared<Audio>(std::move(*this));
}