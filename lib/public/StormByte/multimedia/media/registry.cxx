#include <StormByte/multimedia/media/registry.hxx>
#include <StormByte/util/string.hxx>

#include <ranges>

using namespace StormByte::Multimedia::Media;

const std::vector<Codec::Info::PointerType> Registry::c_codec_registry = {
	// Audio codecs
	std::make_shared<Codec::Info>(Codec::ID::AAC,			Type::Audio, 		"AAC"			),
	std::make_shared<Codec::Info>(Codec::ID::AC3,			Type::Audio, 		"AC3"			),
	std::make_shared<Codec::Info>(Codec::ID::DTS,			Type::Audio, 		"DTS"			),
	std::make_shared<Codec::Info>(Codec::ID::EAC3,		Type::Audio, 		"EAC3"			),
	std::make_shared<Codec::Info>(Codec::ID::FLAC,		Type::Audio, 		"FLAC"			),
	std::make_shared<Codec::Info>(Codec::ID::MP3,			Type::Audio, 		"MP3"			),
	std::make_shared<Codec::Info>(Codec::ID::OPUS,		Type::Audio, 		"OPUS"			),
	std::make_shared<Codec::Info>(Codec::ID::PCM,			Type::Audio, 		"PCM"			),
	std::make_shared<Codec::Info>(Codec::ID::VORBIS,		Type::Audio, 		"VORBIS"		),
	std::make_shared<Codec::Info>(Codec::ID::WMA,			Type::Audio, 		"WMA"			),
	std::make_shared<Codec::Info>(Codec::ID::ALAC,		Type::Audio, 		"ALAC"			),
	std::make_shared<Codec::Info>(Codec::ID::MPEG1L2,		Type::Audio, 		"MPEG1L2"		),

	// Video codecs
	std::make_shared<Codec::Info>(Codec::ID::AV1,			Type::Video,		"AV1"			),
	std::make_shared<Codec::Info>(Codec::ID::AVC,			Type::Video,		"AVC"			),
	std::make_shared<Codec::Info>(Codec::ID::H264,		Type::Video,		"H264"			),
	std::make_shared<Codec::Info>(Codec::ID::H265,		Type::Video,		"HEVC"			),
	std::make_shared<Codec::Info>(Codec::ID::MJPEG,		Type::Video,		"MJPEG"			),
	std::make_shared<Codec::Info>(Codec::ID::THEORA,		Type::Video,		"THEORA"		),
	std::make_shared<Codec::Info>(Codec::ID::VP8,			Type::Video,		"VP8"			),
	std::make_shared<Codec::Info>(Codec::ID::VP9,			Type::Video,		"VP9"			),
	std::make_shared<Codec::Info>(Codec::ID::XVID,		Type::Video,		"XVID"			),

	// Subtitle codecs
	std::make_shared<Codec::Info>(Codec::ID::ASUB,		Type::Subtitle,		"ASUB"			),
	std::make_shared<Codec::Info>(Codec::ID::SUBRIP,		Type::Subtitle,		"SUBRIP"		),
	std::make_shared<Codec::Info>(Codec::ID::WEBVTT,		Type::Subtitle,		"WEBVTT"		),

	// Image codecs
	std::make_shared<Codec::Info>(Codec::ID::BMP,			Type::Image,		"BMP"			),
	std::make_shared<Codec::Info>(Codec::ID::GIF,			Type::Image,		"GIF"			),
	std::make_shared<Codec::Info>(Codec::ID::JPEG,		Type::Image,		"JPEG"			),
	std::make_shared<Codec::Info>(Codec::ID::PNG,			Type::Image,		"PNG"			),
	std::make_shared<Codec::Info>(Codec::ID::TIFF,		Type::Image,		"TIFF"			),
	std::make_shared<Codec::Info>(Codec::ID::JPEG_XL,		Type::Image,		"JPEG_XL"		)
};

const std::unordered_map<std::string, Codec::Info::PointerType> Registry::c_codec_name_map = []() -> std::unordered_map<std::string, Codec::Info::PointerType> {
	std::unordered_map<std::string, Codec::Info::PointerType> map;
	for (auto codec_ptr : c_codec_registry) {
		map[StormByte::Util::String::ToLower(codec_ptr->NameToString())] = codec_ptr;
	}
	return map;
}();

Codec::Info::PointerType Registry::CodecInfo(const Codec::ID& codec) {
	const auto it = std::ranges::find_if(c_codec_registry, [codec](const Codec::Info::PointerType& codec_ptr) {
		return codec_ptr->Name() == codec;
	});

	return (it != c_codec_registry.end()) ? *it : nullptr;
}

StormByte::Expected<Codec::Info::PointerType, StormByte::Multimedia::CodecNotFound> Registry::CodecInfo(const std::string& name) {
	auto it = c_codec_name_map.find(StormByte::Util::String::ToLower(name));
	if (it != c_codec_name_map.end())
		return it->second;

	return Unexpected<CodecNotFound>(name);
}

const std::vector<Container::Info::PointerType> Registry::c_container_registry = {
    // Audio Containers
    std::make_shared<Container::Info>(Container::AC3,  	"AC3",		std::vector<Codec::ID>{ Codec::AC3 														} ),
    std::make_shared<Container::Info>(Container::FLAC, 	"FLAC",		std::vector<Codec::ID>{ Codec::FLAC 														} ),
    std::make_shared<Container::Info>(Container::MP3,  	"MP3",		std::vector<Codec::ID>{ Codec::MP3 														} ),
    std::make_shared<Container::Info>(Container::OGA,  	"OGA",		std::vector<Codec::ID>{ Codec::VORBIS, Codec::OPUS 										} ),
    std::make_shared<Container::Info>(Container::WAV,  	"WAV",		std::vector<Codec::ID>{ Codec::PCM, Codec::WMA 																							}		),

    // Video Containers
    std::make_shared<Container::Info>(Container::AVI,  	"AVI",		std::vector<Codec::ID>{ Codec::H264, Codec::MJPEG, Codec::XVID 							} ),
    std::make_shared<Container::Info>(Container::MKV,  	"MKV",		std::vector<Codec::ID>{ Codec::H264, Codec::H265, Codec::VP8, Codec::VP9, Codec::AV1,
																								Codec::AAC, Codec::FLAC, Codec::VORBIS, Codec::SUBRIP,
																								Codec::WEBVTT, Codec::OPUS 										} ),
    std::make_shared<Container::Info>(Container::MP4,  	"MP4",		std::vector<Codec::ID>{ Codec::H264, Codec::H265, Codec::AV1, Codec::AAC, Codec::FLAC,
																								Codec::MP3, Codec::EAC3											} ),
    std::make_shared<Container::Info>(Container::M2TS, 	"M2TS",		std::vector<Codec::ID>{ Codec::H264, Codec::H265, Codec::AC3, Codec::AAC, Codec::PCM		} ),
    std::make_shared<Container::Info>(Container::MPEG, 	"MPEG",		std::vector<Codec::ID>{ Codec::H264, Codec::MP3 											} ),
    std::make_shared<Container::Info>(Container::MPG,  	"MPG",		std::vector<Codec::ID>{ Codec::H264, Codec::MP3											} ),
    std::make_shared<Container::Info>(Container::OGV,  	"OGV",		std::vector<Codec::ID>{ Codec::THEORA, Codec::VORBIS 										} ),
    std::make_shared<Container::Info>(Container::TS,   	"TS",		std::vector<Codec::ID>{ Codec::H264, Codec::H265, Codec::AAC 								} ),
    std::make_shared<Container::Info>(Container::WEBM, 	"WEBM",		std::vector<Codec::ID>{ Codec::VP8, Codec::VP9, Codec::OPUS								} ),

    // Image Containers
    std::make_shared<Container::Info>(Container::BMP,  	"BMP",		std::vector<Codec::ID>{ Codec::BMP 														} ),
    std::make_shared<Container::Info>(Container::GIF,  	"GIF",		std::vector<Codec::ID>{ Codec::GIF 														} ),
    std::make_shared<Container::Info>(Container::HEIC, 	"HEIC",		std::vector<Codec::ID>{ Codec::TIFF 														} ),
    std::make_shared<Container::Info>(Container::JPG,  	"JPG",		std::vector<Codec::ID>{ Codec::JPEG 														} ),
    std::make_shared<Container::Info>(Container::PNG,  	"PNG",		std::vector<Codec::ID>{ Codec::PNG 														} )
};

const std::unordered_map<std::string, Container::Info::PointerType> Registry::c_container_name_map = []() -> std::unordered_map<std::string, Container::Info::PointerType> {
    std::unordered_map<std::string, Container::Info::PointerType> map;
    for (auto container_ptr: Registry::c_container_registry) {
        map[StormByte::Util::String::ToLower(container_ptr->NameToString())] = container_ptr;
    }
    return map;
}();

Container::Info::PointerType Registry::ContainerInfo(const Container::Name& container) {
    const auto it = std::ranges::find_if(c_container_registry, [container](const Container::Info::PointerType& container_ptr) {
		return container_ptr->Name() == container;
	});

	return (it != c_container_registry.end()) ? *it : nullptr;
}

StormByte::Expected<Container::Info::PointerType, StormByte::Multimedia::ContainerNotFound> Registry::ContainerInfo(const std::string& name) {
    auto it = c_container_name_map.find(StormByte::Util::String::ToLower(name));
    if (it != c_container_name_map.end())
		return it->second;

    return Unexpected<StormByte::Multimedia::ContainerNotFound>(name);
}

