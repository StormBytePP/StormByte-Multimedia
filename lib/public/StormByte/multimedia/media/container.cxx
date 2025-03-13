#include <StormByte/multimedia/exception.hxx>
#include <StormByte/multimedia/media/container.hxx>
#include <StormByte/util/string.hxx>

#include <unordered_map>
#include <algorithm>

using namespace StormByte::Multimedia::Media;

const std::unordered_map<Container::Name, Container::Info> Container::Registry::c_container_registry = {
    // Audio Containers
    { Container::AC3,  { "AC3",		1,				{ Codec::AC3 } } },
    { Container::FLAC, { "FLAC",	1,				{ Codec::FLAC } } },
    { Container::MP3,  { "MP3",		1,				{ Codec::MP3 } } },
    { Container::OGA,  { "OGA",		std::nullopt,	{ Codec::VORBIS, Codec::OPUS } } },
    { Container::WAV,  { "WAV",		1,				{ Codec::PCM, Codec::WMA } } },

    // Video Containers
    { Container::AVI,  { "AVI",		1,				{ Codec::H264, Codec::MJPEG, Codec::XVID } } },
    { Container::MKV,  { "MKV",		std::nullopt,	{ Codec::H264, Codec::H265, Codec::VP8, Codec::VP9, Codec::AV1, Codec::AAC, Codec::FLAC, Codec::VORBIS, Codec::SUBRIP, Codec::WEBVTT, Codec::OPUS } } },
    { Container::MP4,  { "MP4",		std::nullopt,	{ Codec::H264, Codec::H265, Codec::AV1, Codec::AAC, Codec::FLAC, Codec::MP3, Codec::EAC3 } } },
    { Container::M2TS, { "M2TS",	std::nullopt,	{ Codec::H264, Codec::H265, Codec::AC3, Codec::AAC, Codec::PCM } } },
    { Container::MPEG, { "MPEG",	std::nullopt,	{ Codec::H264, Codec::MP3 } } },
    { Container::MPG,  { "MPG",		std::nullopt,	{ Codec::H264, Codec::MP3 } } },
    { Container::OGV,  { "OGV",		std::nullopt,	{ Codec::THEORA, Codec::VORBIS } } },
    { Container::TS,   { "TS",		std::nullopt,	{ Codec::H264, Codec::H265, Codec::AAC } } },
    { Container::WEBM, { "WEBM",	std::nullopt,	{ Codec::VP8, Codec::VP9, Codec::OPUS } } },

    // Image Containers
    { Container::BMP,  { "BMP",		1,				{ Codec::BMP } } },
    { Container::GIF,  { "GIF",		1,				{ Codec::GIF } } },
    { Container::HEIC, { "HEIC",	1,				{ Codec::TIFF } } },
    { Container::JPG,  { "JPG",		1,				{ Codec::JPEG } } },
    { Container::PNG,  { "PNG",		1,				{ Codec::PNG } } },
};

const std::unordered_map<std::string, Container::Name> Container::Registry::c_container_name_map = [] {
    std::unordered_map<std::string, Container::Name> map;
    for (const auto& [container, info] : Registry::c_container_registry) {
        map[StormByte::Util::String::ToLower(info.s_name)] = container;
    }
    return map;
}();

const Container::Info& Container::Registry::Info(const Container::Name& container) {
    return c_container_registry.at(container);
}

const Container::Name& Container::Registry::Info(const std::string& name) {
    auto it = c_container_name_map.find(StormByte::Util::String::ToLower(name));
    if (it == c_container_name_map.end()) {
        throw StormByte::Multimedia::ContainerNotFound(name);
    }
    return it->second;
}
