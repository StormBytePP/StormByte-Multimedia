#include <multimedia/media/duration.hxx>

using namespace StormByte::Multimedia::Media;

std::string Duration::GetName() const noexcept {
	return std::to_string(s_secs / 3600) + ":" + std::to_string((s_secs % 3600) / 60) + ":" + std::to_string(s_secs % 60);
}