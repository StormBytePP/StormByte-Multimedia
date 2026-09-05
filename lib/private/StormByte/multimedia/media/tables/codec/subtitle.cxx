#include <StormByte/multimedia/media/tables/codec/table.hxx>

using namespace StormByte::Multimedia::Media::Tables::Codec;

namespace {
	constexpr CodecDef table[] = {
		{ "3GPP Timed Text",	"MPEG-4 / 3GPP timed text",						{ "mov_text" } },
		{ "ARIB Caption",		"ARIB STD-B24 caption",							{ "arib_caption" } },
		{ "ASS",				"Advanced SubStation Alpha",					{ "ass" } },
		{ "DVB Subtitle",		"DVB subtitle",									{ "dvb_subtitle" } },
		{ "DVB Teletext",		"DVB teletext",									{ "dvb_teletext" } },
		{ "DVD Subtitle",		"DVD subtitle",									{ "dvd_subtitle" } },
		{ "EIA-608",			"CEA/EIA-608 captions",							{ "eia_608" } },
		{ "HDMV Text",			"HDMV TextST",									{ "hdmv_text_subtitle" } },
		{ "IVTV VBI",			"IVTV VBI",										{ "ivtv_vbi" } },
		{ "JACOsub",			"JACOsub",										{ "jacosub" } },
		{ "MPL2",				"MPL2",											{ "mpl2" } },
		{ "MicroDVD",			"MicroDVD",										{ "microdvd" } },
		{ "PGS",				"HDMV Presentation Graphic Stream",				{ "hdmv_pgs_subtitle" } },
		{ "PJS",				"PJS",											{ "pjs" } },
		{ "RealText",			"RealText",										{ "realtext" } },
		{ "SAMI",				"Synchronized Accessible Media Interchange",	{ "sami" } },
		{ "SRT",				"SubRip (srt)",									{ "srt" } },
		{ "SSA",				"SubStation Alpha",								{ "ssa" } },
		{ "Spruce STL",			"Spruce subtitle",								{ "stl" } },
		{ "SubRip",				"SubRip",										{ "subrip" } },
		{ "SubViewer",			"SubViewer v2",									{ "subviewer" } },
		{ "SubViewer 1",		"SubViewer v1",									{ "subviewer1" } },
		{ "TTML",				"Timed Text Markup Language",					{ "ttml" } },
		{ "UTF-8 Text",			"Raw UTF-8 text",								{ "text" } },
		{ "VPlayer",			"VPlayer",										{ "vplayer" } },
		{ "WebVTT",				"WebVTT",										{ "webvtt" } },
		{ "XSUB",				"DivX XSUB",									{ "xsub" } },
	};
}

std::span<const CodecDef> StormByte::Multimedia::Media::Tables::Codec::Subtitle() noexcept {
	return table;
}
