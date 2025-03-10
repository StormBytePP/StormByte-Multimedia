#include <multimedia/exception.hxx>
#include <multimedia/media/container.hxx>
#include <util/string.hxx>

#include <unordered_map>
#include <algorithm>

using namespace StormByte::Multimedia::Media;

const std::unordered_map<Container::Name, Container::Info> Container::Registry::c_container_registry = {
    // Audio Containers
    { Container::Name::AC3,  { "AC3",	1,				{ Codec::Name::AC3 } } },
    { Container::Name::FLAC, { "FLAC",	1,				{ Codec::Name::FLAC } } },
    { Container::Name::MP3,  { "MP3",	1,				{ Codec::Name::MP3 } } },
    { Container::Name::OGA,  { "OGA",	std::nullopt,	{ Codec::Name::VORBIS, Codec::Name::OPUS } } },
    { Container::Name::WAV,  { "WAV",	1,				{ Codec::Name::PCM, Codec::Name::WMA } } },

    // Video Containers
    { Container::Name::AVI,  { "AVI",	1,				{ Codec::Name::H264, Codec::Name::MJPEG, Codec::Name::XVID } } },
    { Container::Name::MKV,  { "MKV",	std::nullopt,	{ Codec::Name::H264, Codec::Name::H265, Codec::Name::VP8, Codec::Name::VP9, Codec::Name::AV1, Codec::Name::AAC, Codec::Name::FLAC, Codec::Name::VORBIS, Codec::Name::SUBRIP, Codec::Name::WEBVTT, Codec::Name::OPUS } } },
    { Container::Name::MP4,  { "MP4",	std::nullopt,	{ Codec::Name::H264, Codec::Name::H265, Codec::Name::AV1, Codec::Name::AAC, Codec::Name::FLAC, Codec::Name::MP3 } } },
    { Container::Name::M2TS, { "M2TS",	std::nullopt,	{ Codec::Name::H264, Codec::Name::H265, Codec::Name::AC3, Codec::Name::AAC, Codec::Name::PCM } } },
    { Container::Name::MPEG, { "MPEG",	std::nullopt,	{ Codec::Name::H264, Codec::Name::MP3, Codec::Name::AAC } } },
    { Container::Name::MPG,  { "MPG",	std::nullopt,	{ Codec::Name::H264, Codec::Name::MP3, Codec::Name::AAC } } },
    { Container::Name::OGV,  { "OGV",	std::nullopt,	{ Codec::Name::THEORA, Codec::Name::VORBIS } } },
    { Container::Name::TS,   { "TS",	std::nullopt,	{ Codec::Name::H264, Codec::Name::H265, Codec::Name::AAC } } },
    { Container::Name::WEBM, { "WEBM",	std::nullopt,	{ Codec::Name::VP8, Codec::Name::VP9, Codec::Name::OPUS } } },

    // Image Containers
    { Container::Name::BMP,  { "BMP",	1,				{ Codec::Name::BMP } } },
    { Container::Name::GIF,  { "GIF",	1,				{ Codec::Name::GIF } } },
    { Container::Name::HEIC, { "HEIC",	1,				{ Codec::Name::TIFF } } },
    { Container::Name::JPG,  { "JPG",	1,				{ Codec::Name::JPEG } } },
    { Container::Name::PNG,  { "PNG",	1,				{ Codec::Name::PNG } } },
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
