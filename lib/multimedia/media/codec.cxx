#include <multimedia/exception.hxx>
#include <multimedia/media/codec.hxx>

#include <unordered_map>

using namespace StormByte::Multimedia::Media;

Codec CodecByName(const std::string& name) {
	static const std::unordered_map<std::string, Codec> codecMap = {
		// Audio codecs
		{"aac", Codec::AAC},
		{"ac3", Codec::AC3},
		{"dts", Codec::DTS},
		{"eac3", Codec::EAC3},
		{"flac", Codec::FLAC},
		{"mp3", Codec::MP3},
		{"opus", Codec::OPUS},
		{"pcm", Codec::PCM},
		{"vorbis", Codec::VORBIS},
		{"wma", Codec::WMA},

		// Video codecs
		{"av1", Codec::AV1},
		{"avc", Codec::AVC},
		{"h264", Codec::H264},
		{"h265", Codec::H265},
		{"mjpeg", Codec::MJPEG},
		{"theora", Codec::THEORA},
		{"vp8", Codec::VP8},
		{"vp9", Codec::VP9},
		{"xvid", Codec::XVID},

		// Subtitle codecs
		{"asub", Codec::ASUB},
		{"subrip", Codec::SUBRIP},
		{"webvtt", Codec::WEBVTT},

		// Image codecs
		{"bmp", Codec::BMP},
		{"gif", Codec::GIF},
		{"jpeg", Codec::JPEG},
		{"png", Codec::PNG},
		{"tiff", Codec::TIFF}
	};

	auto it = codecMap.find(name);
	if (it != codecMap.end()) {
		return it->second;
	}

	throw StormByte::Multimedia::CodecNotFound(name); // Handle unknown codec
}	