#include <StormByte/multimedia/media/property/audio/channels.hxx>

using namespace StormByte::Multimedia::Media::Property::Audio;

Channels::Channels(const unsigned short& number, const std::string& layout) noexcept: m_number(number), m_layout(layout) {}

Channels::Channels(unsigned short&& number, std::string&& layout) noexcept: m_number(std::move(number)), m_layout(std::move(layout)) {}

const unsigned short& Channels::Number() const noexcept {
	return m_number;
}

const std::string& Channels::Layout() const noexcept {
	return m_layout;
}