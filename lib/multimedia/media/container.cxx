#include <multimedia/media/container.hxx>
#include <util/string.hxx>

#include <algorithm>
#include <unordered_map>

using namespace StormByte::Multimedia::Media;

Container STORMBYTE_MULTIMEDIA_PUBLIC ContainerFromExtension(const std::string& extension) noexcept {
	// Define the mapping as a constexpr unordered_map, grouped and sorted by category
	const std::unordered_map<std::string_view, Container> containerMap = {
		// Audio
		{	".ac3", 	Container::AC3		},
		{	".flac", 	Container::FLAC		},
		{	".mp3", 	Container::MP3		},
		{	".oga", 	Container::OGA		},
		{	".wav", 	Container::WAV		},

		// Video
		{	".avi", 	Container::AVI		},
		{	".m2ts", 	Container::M2TS		},
		{	".mkv", 	Container::MKV		},
		{	".mp4", 	Container::MP4		},
		{	".mpeg", 	Container::MPEG		},
		{	".mpg", 	Container::MPG		},
		{	".ogv", 	Container::OGV		},
		{	".ts", 		Container::TS		},
		{	".webm", 	Container::WEBM		},

		// Image
		{	".bmp", 	Container::BMP		},
		{	".gif", 	Container::GIF		},
		{	".heic", 	Container::HEIC		},
		{	".jpg", 	Container::JPG		},
		{	".png", 	Container::PNG		}
	};

	// Search in the map
	auto it = containerMap.find(StormByte::Util::String::ToLower(extension));
	return (it != containerMap.end()) ? it->second : Container::Unknown;
}