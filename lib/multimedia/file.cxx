#include <multimedia/file.hxx>

using namespace StormByte::Multimedia;

File::File(const std::filesystem::path& path):m_path(path) {
	std::error_code ec; // Create an error_code object
	auto size = std::filesystem::file_size(path, ec);
	if (ec) {
		m_size.s_bytes = 0;
	}
	else {
		m_size.s_bytes = size;
	}
	m_container = Container::Base::Create(path.extension().string());
}

const std::filesystem::path& File::GetPath() const noexcept {
	return m_path;
}

const Property::Size& File::GetSize() const noexcept {
	return m_size;
}

std::shared_ptr<Container::Base> File::GetContainer() const noexcept {
	return m_container;
}