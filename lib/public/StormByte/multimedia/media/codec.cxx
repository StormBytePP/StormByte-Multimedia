#include <StormByte/multimedia/media/codec.hxx>
#include <StormByte/util/string.hxx>

#include <unordered_map>

using namespace StormByte::Multimedia::Media;

const std::unordered_map<Codec::Name, Codec::Info> Codec::Registry::c_codec_registry = {
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

const std::unordered_map<std::string, Codec::Name> Codec::Registry::c_codec_name_map = [] {
    std::unordered_map<std::string, Codec::Name> map;
    for (const auto& [codec, info] : c_codec_registry) {
        map[StormByte::Util::String::ToLower(info.s_name)] = codec;
    }
    return map;
}();

const Codec::Info& Codec::Registry::Info(const Codec::Name& codec) {
	return c_codec_registry.at(codec);
}

StormByte::Expected<Codec::Name, StormByte::Multimedia::CodecNotFound> Codec::Registry::Info(const std::string& name) {
	auto it = c_codec_name_map.find(StormByte::Util::String::ToLower(name));
	if (it != c_codec_name_map.end())
		return it->second;

	return Unexpected<CodecNotFound>(name);
}