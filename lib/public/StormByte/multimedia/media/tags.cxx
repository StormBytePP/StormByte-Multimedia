#include <StormByte/multimedia/media/tags.hxx>

#include <format>

using namespace StormByte::Multimedia::Media;

std::string& Tags::operator[](const std::string& key) noexcept {
	return m_tags[key];
}

StormByte::Expected<const std::string&, StormByte::Multimedia::Exception> Tags::operator[](const std::string& key) const noexcept {
	try {
		return m_tags.at(key);
	} catch (const std::out_of_range&) {
		return StormByte::Unexpected<Multimedia::Exception>(std::format("Tag {} not found", key));
	}
}

void Tags::Remove(const std::string& key) noexcept {
	m_tags.erase(key);
}

Tags::iterator Tags::begin() noexcept {
	return m_tags.begin();
}

Tags::const_iterator Tags::begin() const noexcept {
	return m_tags.begin();
}

Tags::iterator Tags::end() noexcept {
	return m_tags.end();
}

Tags::const_iterator Tags::end() const noexcept {
	return m_tags.end();
}