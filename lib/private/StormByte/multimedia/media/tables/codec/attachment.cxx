#include <StormByte/multimedia/media/tables/codec/table.hxx>

using namespace StormByte::Multimedia::Media::Tables::Codec;

namespace {
	constexpr CodecDef table[] = {
		{ "Binary Data",		"Binary data",								{ "bin_data" } },
		{ "Binary Text",		"Binary text",								{ "bintext" } },
		{ "DVD Navigation",		"DVD navigation packets",					{ "dvd_nav_packet" } },
		{ "EPG",				"Electronic Program Guide",					{ "epg" } },
		{ "ITU-T T.35",			"ITU-T T.35",								{ "itut_t35" } },
		{ "LCEVC",				"Low Complexity Enhancement Video Coding",	{ "lcevc" } },
		{ "OpenType Font",		"OpenType font",							{ "otf" } },
		{ "SCTE-35",			"SCTE-35",									{ "scte_35" } },
		{ "SMPTE 2038",			"SMPTE 2038",								{ "smpte_2038" } },
		{ "SMPTE 436M ANC",		"SMPTE 436M ANC",							{ "smpte_436m_anc" } },
		{ "SMPTE KLV",			"SMPTE 336M KLV",							{ "smpte_klv" } },
		{ "Timed ID3",			"Timed ID3 metadata",						{ "timed_id3" } },
		{ "TrueType Font",		"TrueType font",							{ "ttf" } },
		{ "XBIN",				"eXtended BIN",								{ "xbin" } },
		{ "iCEDraw",			"iCEDraw file",								{ "idf" } },
	};
}

std::span<const CodecDef> StormByte::Multimedia::Media::Tables::Codec::Attachment() noexcept {
	return table;
}
