#include <StormByte/multimedia/media/type.hxx>

namespace StormByte::Multimedia::Media {
	std::string TypeToString(Type type) {
		switch (type) {
			case Type::Audio:
				return "Audio";
			case Type::Video:
				return "Video";
			case Type::Subtitle:
				return "Subtitle";
			case Type::Image:
				return "Image";
			case Type::Attachment:
				return "Attachment";
			default:
				return "Unknown";
			}
	}
}