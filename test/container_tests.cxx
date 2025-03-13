#include <StormByte/multimedia/container.hxx>
#include <StormByte/multimedia/stream.hxx>

#include "test_handlers.h"

using namespace StormByte::Multimedia;

// Test cases for all containers

// AC3 Container
int test_ac3_container_codecs() {
    auto ac3 = Container(Media::Container::AC3);
    ASSERT_EQUAL("test_ac3_container_codecs", true, ac3.Supports(Media::Codec::AC3));
    ASSERT_EQUAL("test_ac3_container_codecs", false, ac3.Supports(Media::Codec::H264));
    RETURN_TEST("test_ac3_container_codecs", 0);
}

// FLAC Container
int test_flac_container_codecs() {
    auto flac = Container(Media::Container::FLAC);
    ASSERT_EQUAL("test_flac_container_codecs", true, flac.Supports(Media::Codec::FLAC));
    ASSERT_EQUAL("test_flac_container_codecs", false, flac.Supports(Media::Codec::MP3));
    RETURN_TEST("test_flac_container_codecs", 0);
}

// MP3 Container
int test_mp3_container_codecs() {
    auto mp3 = Container(Media::Container::MP3);
    ASSERT_EQUAL("test_mp3_container_codecs", true, mp3.Supports(Media::Codec::MP3));
    ASSERT_EQUAL("test_mp3_container_codecs", false, mp3.Supports(Media::Codec::AAC));
    RETURN_TEST("test_mp3_container_codecs", 0);
}

// OGA Container
int test_oga_container_codecs() {
    auto oga = Container(Media::Container::OGA);
    ASSERT_EQUAL("test_oga_container_codecs", true, oga.Supports(Media::Codec::VORBIS) && oga.Supports(Media::Codec::OPUS));
    ASSERT_EQUAL("test_oga_container_codecs", false, oga.Supports(Media::Codec::H264));
    RETURN_TEST("test_oga_container_codecs", 0);
}

// WAV Container
int test_wav_container_codecs() {
    auto wav = Container(Media::Container::WAV);
    ASSERT_EQUAL("test_wav_container_codecs", true, wav.Supports(Media::Codec::PCM) && wav.Supports(Media::Codec::WMA));
    ASSERT_EQUAL("test_wav_container_codecs", false, wav.Supports(Media::Codec::MP3));
    RETURN_TEST("test_wav_container_codecs", 0);
}

// AVI Container
int test_avi_container_codecs() {
    auto avi = Container(Media::Container::AVI);
    ASSERT_EQUAL("test_avi_container_codecs", true, avi.Supports(Media::Codec::H264) && avi.Supports(Media::Codec::MJPEG));
    ASSERT_EQUAL("test_avi_container_codecs", false, avi.Supports(Media::Codec::AAC));
    RETURN_TEST("test_avi_container_codecs", 0);
}

// MKV Container
int test_mkv_container_codecs() {
    auto mkv = Container(Media::Container::MKV);
    ASSERT_EQUAL("test_mkv_container_codecs", true,
        mkv.Supports(Media::Codec::H264) &&
        mkv.Supports(Media::Codec::H265) &&
        mkv.Supports(Media::Codec::VP8) &&
        mkv.Supports(Media::Codec::AAC));
    ASSERT_EQUAL("test_mkv_container_codecs", false, mkv.Supports(Media::Codec::MP3));
    RETURN_TEST("test_mkv_container_codecs", 0);
}

// MP4 Container
int test_mp4_container_codecs() {
    auto mp4 = Container(Media::Container::MP4);
    ASSERT_EQUAL("test_mp4_container_codecs", true,
        mp4.Supports(Media::Codec::H264) &&
        mp4.Supports(Media::Codec::H265) &&
        mp4.Supports(Media::Codec::AAC));
    ASSERT_EQUAL("test_mp4_container_codecs", false, mp4.Supports(Media::Codec::VORBIS));
    RETURN_TEST("test_mp4_container_codecs", 0);
}

// M2TS Container
int test_m2ts_container_codecs() {
    auto m2ts = Container(Media::Container::M2TS);
    ASSERT_EQUAL("test_m2ts_container_codecs", true,
        m2ts.Supports(Media::Codec::H264) &&
        m2ts.Supports(Media::Codec::H265) &&
        m2ts.Supports(Media::Codec::AC3));
    ASSERT_EQUAL("test_m2ts_container_codecs", false, m2ts.Supports(Media::Codec::VP8));
    RETURN_TEST("test_m2ts_container_codecs", 0);
}

// MPEG Container
int test_mpeg_container_codecs() {
    auto mpeg = Container(Media::Container::MPEG);
    ASSERT_EQUAL("test_mpeg_container_codecs", true,
        mpeg.Supports(Media::Codec::H264) &&
        mpeg.Supports(Media::Codec::MP3));
    ASSERT_EQUAL("test_mpeg_container_codecs", false, mpeg.Supports(Media::Codec::VORBIS));
    RETURN_TEST("test_mpeg_container_codecs", 0);
}

// MPG Container
int test_mpg_container_codecs() {
    auto mpg = Container(Media::Container::MPG);
    ASSERT_EQUAL("test_mpg_container_codecs", true,
        mpg.Supports(Media::Codec::H264) &&
        mpg.Supports(Media::Codec::MP3));
    ASSERT_EQUAL("test_mpg_container_codecs", false, mpg.Supports(Media::Codec::AAC));
    RETURN_TEST("test_mpg_container_codecs", 0);
}

// OGV Container
int test_ogv_container_codecs() {
    auto ogv = Container(Media::Container::OGV);
    ASSERT_EQUAL("test_ogv_container_codecs", true,
        ogv.Supports(Media::Codec::THEORA) &&
        ogv.Supports(Media::Codec::VORBIS));
    ASSERT_EQUAL("test_ogv_container_codecs", false, ogv.Supports(Media::Codec::H264));
    RETURN_TEST("test_ogv_container_codecs", 0);
}

// TS Container
int test_ts_container_codecs() {
    auto ts = Container(Media::Container::TS);
    ASSERT_EQUAL("test_ts_container_codecs", true,
        ts.Supports(Media::Codec::H264) &&
        ts.Supports(Media::Codec::AAC));
    ASSERT_EQUAL("test_ts_container_codecs", false, ts.Supports(Media::Codec::VP9));
    RETURN_TEST("test_ts_container_codecs", 0);
}

// WEBM Container
int test_webm_container_codecs() {
    auto webm = Container(Media::Container::WEBM);
    ASSERT_EQUAL("test_webm_container_codecs", true,
        webm.Supports(Media::Codec::VP8) &&
        webm.Supports(Media::Codec::VP9) &&
        webm.Supports(Media::Codec::OPUS));
    ASSERT_EQUAL("test_webm_container_codecs", false, webm.Supports(Media::Codec::AAC));
    RETURN_TEST("test_webm_container_codecs", 0);
}

// BMP Container
int test_bmp_container_codecs() {
    auto bmp = Container(Media::Container::BMP);
    ASSERT_EQUAL("test_bmp_container_codecs", true, bmp.Supports(Media::Codec::BMP));
    ASSERT_EQUAL("test_bmp_container_codecs", false, bmp.Supports(Media::Codec::JPEG));
    RETURN_TEST("test_bmp_container_codecs", 0);
}

// GIF Container
int test_gif_container_codecs() {
    auto gif = Container(Media::Container::GIF);
    ASSERT_EQUAL("test_gif_container_codecs", true, gif.Supports(Media::Codec::GIF));
    ASSERT_EQUAL("test_gif_container_codecs", false, gif.Supports(Media::Codec::PNG));
    RETURN_TEST("test_gif_container_codecs", 0);
}

// HEIC Container
int test_heic_container_codecs() {
    auto heic = Container(Media::Container::HEIC);
    ASSERT_EQUAL("test_heic_container_codecs", true, heic.Supports(Media::Codec::TIFF));
    ASSERT_EQUAL("test_heic_container_codecs", false, heic.Supports(Media::Codec::JPEG));
    RETURN_TEST("test_heic_container_codecs", 0);
}

// JPG Container
int test_jpg_container_codecs() {
    auto jpg = Container(Media::Container::JPG);
    ASSERT_EQUAL("test_jpg_container_codecs", true, jpg.Supports(Media::Codec::JPEG));
    ASSERT_EQUAL("test_jpg_container_codecs", false, jpg.Supports(Media::Codec::PNG));
    RETURN_TEST("test_jpg_container_codecs", 0);
}

// PNG Container
int test_png_container_codecs() {
    auto png = Container(Media::Container::PNG);
    ASSERT_EQUAL("test_png_container_codecs", true, png.Supports(Media::Codec::PNG));
    ASSERT_EQUAL("test_png_container_codecs", false, png.Supports(Media::Codec::GIF));
    RETURN_TEST("test_png_container_codecs", 0);
}

// Main function
int main() {
    int result = 0;
    try {
        result += test_ac3_container_codecs();
        result += test_flac_container_codecs();
        result += test_mp3_container_codecs();
        result += test_oga_container_codecs();
        result += test_wav_container_codecs();
        result += test_avi_container_codecs();
        result += test_mkv_container_codecs();
        result += test_mp4_container_codecs();
        result += test_m2ts_container_codecs();
        result += test_mpeg_container_codecs();
		result += test_mpg_container_codecs();
		result += test_ogv_container_codecs();
		result += test_ts_container_codecs();
		result += test_webm_container_codecs();
		result += test_bmp_container_codecs();
		result += test_gif_container_codecs();
		result += test_heic_container_codecs();
		result += test_jpg_container_codecs();
		result += test_png_container_codecs();
		std::cout << "All tests passed successfully.\n";
    } catch (...) {
        result++;
	}

    return result;
}