#include <StormByte/multimedia/media/sample.hxx>

using namespace StormByte::Multimedia::Media;

Sample::Sample(const std::string& format, const unsigned int& rate): m_format(format), m_rate(rate) {}

std::string& Sample::Format() noexcept {
	return m_format;
}

const std::string& Sample::Format() const noexcept {
	return m_format;
}

unsigned int& Sample::Rate() noexcept {
	return m_rate;
}

const unsigned int& Sample::Rate() const noexcept {
	return m_rate;
}