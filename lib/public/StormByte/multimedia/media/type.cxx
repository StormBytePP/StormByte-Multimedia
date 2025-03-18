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
			case Type::Attachment:
				return "Attachment";
			case Type::Unknown:
			default:
				return "Unknown";
			}
	}
}