#include <StormByte/multimedia/media/registry.hxx>
#include <StormByte/util/string.hxx>

using namespace StormByte::Multimedia::Media;

const std::unordered_map<Codec::Name, Codec::Info> Registry::c_codec_registry = {
	// Audio codecs
	{ Codec::Name::AAC,		{ "AAC",		Type::Audio		} },
	{ Codec::Name::AC3,		{ "AC3",		Type::Audio		} },
	{ Codec::Name::DTS,		{ "DTS",		Type::Audio		} },
	{ Codec::Name::EAC3,	{ "EAC3",		Type::Audio		} },
	{ Codec::Name::FLAC,	{ "FLAC",		Type::Audio		} },
	{ Codec::Name::MP3,		{ "MP3",		Type::Audio		} },
	{ Codec::Name::OPUS,	{ "OPUS",		Type::Audio		} },
	{ Codec::Name::PCM,		{ "PCM",		Type::Audio		} },
	{ Codec::Name::VORBIS,	{ "VORBIS",		Type::Audio		} },
	{ Codec::Name::WMA,		{ "WMA",		Type::Audio		} },
	{ Codec::Name::ALAC,	{ "ALAC",		Type::Audio		} },
	{ Codec::Name::MPEG1L2,	{ "MPEG1L2",	Type::Audio		} },

	// Video codecs
	{ Codec::Name::AV1,		{ "AV1",		Type::Video		} },
	{ Codec::Name::AVC,		{ "AVC",		Type::Video		} },
	{ Codec::Name::H264,	{ "H264",		Type::Video		} },
	{ Codec::Name::H265,	{ "HEVC",		Type::Video		} },
	{ Codec::Name::MJPEG,	{ "MJPEG",		Type::Video		} },
	{ Codec::Name::THEORA,	{ "THEORA",		Type::Video		} },
	{ Codec::Name::VP8,		{ "VP8",		Type::Video		} },
	{ Codec::Name::VP9,		{ "VP9",		Type::Video		} },
	{ Codec::Name::XVID,	{ "XVID",		Type::Video		} },

	// Subtitle codecs
	{ Codec::Name::ASUB,	{ "ASUB",		Type::Subtitle	} },
	{ Codec::Name::SUBRIP,	{ "SUBRIP",		Type::Subtitle	} },
	{ Codec::Name::WEBVTT,	{ "WEBVTT",		Type::Subtitle	} },

	// Image codecs
	{ Codec::Name::BMP,		{ "BMP",		Type::Image		} },
	{ Codec::Name::GIF,		{ "GIF",		Type::Image		} },
	{ Codec::Name::JPEG,	{ "JPEG",		Type::Image		} },
	{ Codec::Name::PNG,		{ "PNG",		Type::Image		} },
	{ Codec::Name::TIFF,	{ "TIFF",		Type::Image		} },
	{ Codec::Name::JPEG_XL,	{ "JPEG_XL",	Type::Image		} }
};

const std::unordered_map<std::string, Codec::Name> Registry::c_codec_name_map = [] {
	std::unordered_map<std::string, Codec::Name> map;
	for (const auto& [codec, info] : c_codec_registry) {
		map[StormByte::Util::String::ToLower(info.Name())] = codec;
	}
	return map;
}();

const Codec::Info& Registry::CodecInfo(const Codec::Name& codec) {
	return c_codec_registry.at(codec);
}

StormByte::Expected<Codec::Name, StormByte::Multimedia::CodecNotFound> Registry::CodecInfo(const std::string& name) {
	auto it = c_codec_name_map.find(StormByte::Util::String::ToLower(name));
	if (it != c_codec_name_map.end())
		return it->second;

	return Unexpected<CodecNotFound>(name);
}

const std::unordered_map<Container::Name, Container::Info> Registry::c_container_registry = {
    // Audio Containers
    { Container::AC3,  { "AC3",		{ Codec::AC3 },																																			1 } },
    { Container::FLAC, { "FLAC",	{ Codec::FLAC },																																		1 } },
    { Container::MP3,  { "MP3",		{ Codec::MP3 },																																			1 } },
    { Container::OGA,  { "OGA",		{ Codec::VORBIS, Codec::OPUS } } },
    { Container::WAV,  { "WAV",		{ Codec::PCM, Codec::WMA },																																1 } },

    // Video Containers
    { Container::AVI,  { "AVI",		{ Codec::H264, Codec::MJPEG, Codec::XVID },																												1 } },
    { Container::MKV,  { "MKV",		{ Codec::H264, Codec::H265, Codec::VP8, Codec::VP9, Codec::AV1, Codec::AAC, Codec::FLAC, Codec::VORBIS, Codec::SUBRIP, Codec::WEBVTT, Codec::OPUS } } },
    { Container::MP4,  { "MP4",		{ Codec::H264, Codec::H265, Codec::AV1, Codec::AAC, Codec::FLAC, Codec::MP3, Codec::EAC3 } } },
    { Container::M2TS, { "M2TS",	{ Codec::H264, Codec::H265, Codec::AC3, Codec::AAC, Codec::PCM } } },
    { Container::MPEG, { "MPEG",	{ Codec::H264, Codec::MP3 } } },
    { Container::MPG,  { "MPG",		{ Codec::H264, Codec::MP3 } } },
    { Container::OGV,  { "OGV",		{ Codec::THEORA, Codec::VORBIS } } },
    { Container::TS,   { "TS",		{ Codec::H264, Codec::H265, Codec::AAC } } },
    { Container::WEBM, { "WEBM",	{ Codec::VP8, Codec::VP9, Codec::OPUS } } },

    // Image Containers
    { Container::BMP,  { "BMP",		{ Codec::BMP },																																			1 } },
    { Container::GIF,  { "GIF",		{ Codec::GIF },																																			1 } },
    { Container::HEIC, { "HEIC",	{ Codec::TIFF },																																		1 } },
    { Container::JPG,  { "JPG",		{ Codec::JPEG },																																		1 } },
    { Container::PNG,  { "PNG",		{ Codec::PNG },																																			1 } },
};

const std::unordered_map<std::string, Container::Name> Registry::c_container_name_map = [] {
    std::unordered_map<std::string, Container::Name> map;
    for (const auto& [container, info] : Registry::c_container_registry) {
        map[StormByte::Util::String::ToLower(info.Name())] = container;
    }
    return map;
}();

const Container::Info& Registry::ContainerInfo(const Container::Name& container) {
    return c_container_registry.at(container);
}

StormByte::Expected<Container::Name, StormByte::Multimedia::ContainerNotFound> Registry::ContainerInfo(const std::string& name) {
    auto it = c_container_name_map.find(StormByte::Util::String::ToLower(name));
    if (it != c_container_name_map.end())
	return it->second;

    return Unexpected<StormByte::Multimedia::ContainerNotFound>(name);
}

