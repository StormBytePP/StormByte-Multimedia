// #include <iostream>
// #include <filesystem>
// #include <multimedia/file.hxx>
// #include <multimedia/codec/audio/eac3.hxx>
// #include <multimedia/container/matroska.hxx>
// #include <multimedia/container/avi.hxx>
// #include <multimedia/container/mp4.hxx>
// #include <multimedia/container/webm.hxx>
// #include <multimedia/container/wav.hxx>
// #include <multimedia/container/ogg.hxx>
// #include <multimedia/container/flac.hxx>
// #include <multimedia/container/opus.hxx>
// #include <multimedia/exception.hxx>
// #include <multimedia/stream/audio.hxx>
// #include "test_handlers.h"

// using namespace StormByte::Multimedia;

// // Test cases
// int test_file_initialization() {
//     File file(CurrentFileDirectory / "files" / "test.mp4");
//     ASSERT_FALSE("test_file_initialization", file.GetContainer()->GetType() == Container::Type::Unknown);
//     RETURN_TEST("test_file_initialization", 0);
// }

// int test_invalid_file_path() {
//     try {
// 		// Invalid file does not throw an exception
//         File file(CurrentFileDirectory / "files" / "invalid_file.mkv");
//     } catch (const Exception&) {
//         RETURN_TEST("test_invalid_file_path", 1);
//     }
//     RETURN_TEST("test_invalid_file_path", 0);
// }

// int test_matroska_compatible_codecs() {
//     auto matroska = std::make_shared<Container::Matroska>();
//     auto codecs = matroska->GetCompatibleCodecs();
//     ASSERT_EQUAL("test_matroska_compatible_codecs", true,
//         codecs.contains(Codec::Name::H264) && codecs.contains(Codec::Name::AAC));
//     RETURN_TEST("test_matroska_compatible_codecs", 0);
// }

// int test_unsupported_codec() {
//     auto avi = std::make_shared<Container::AVI>();
//     auto codecs = avi->GetCompatibleCodecs();
//     ASSERT_EQUAL("test_unsupported_codec", false, codecs.contains(Codec::Name::THEORA));
//     RETURN_TEST("test_unsupported_codec", 0);
// }

// int test_codec_count_matroska() {
//     auto matroska = std::make_shared<Container::Matroska>();
//     ASSERT_EQUAL("test_codec_count_matroska", 15, matroska->GetCompatibleCodecs().size());
//     RETURN_TEST("test_codec_count_matroska", 0);
// }

// int test_container_identification() {
//     File file(CurrentFileDirectory / "files" / "test.mkv");
//     ASSERT_FALSE("test_container_identification",
//         std::dynamic_pointer_cast<Container::Matroska>(file.GetContainer()) == nullptr);
//     RETURN_TEST("test_container_identification", 0);
// }

// int test_pcm_wav_compatibility() {
//     auto wav = std::make_shared<Container::WAV>();
//     ASSERT_EQUAL("test_pcm_wav_compatibility", true, wav->GetCompatibleCodecs().contains(Codec::Name::PCM));
//     RETURN_TEST("test_pcm_wav_compatibility", 0);
// }

// int test_empty_file_exception() {
//     try {
// 		// Invalid file does not throw, neither empty
//         File empty_file("");
//     } catch (const Exception&) {
//         RETURN_TEST("test_empty_file_exception", 1);
//     }
//     RETURN_TEST("test_empty_file_exception", 0);
// }

// int test_avi_h264_codec() {
//     auto avi = std::make_shared<Container::AVI>();
//     ASSERT_EQUAL("test_avi_h264_codec", true, avi->GetCompatibleCodecs().contains(Codec::Name::H264));
//     RETURN_TEST("test_avi_h264_codec", 0);
// }

// int test_add_stream_with_codec() {
//     auto container = std::make_unique<Container::MP4>();
    
// 	// Create EAC3 codec
// 	Codec::Audio::EAC3 eac3;
//     // Create a stream with a valid codec
//     Stream::Audio stream(std::move(eac3), Property::Duration(100));

//     // Add the stream to the container
//     container->AddStream(std::move(stream));

//     // Check if the stream was successfully added and its codec is compatible
//    	auto streams = container->GetStreams();
//     ASSERT_EQUAL("test_add_stream_with_codec", 1, container->GetStreamCount());
//     ASSERT_EQUAL("test_add_stream_with_codec", Codec::NameToString(Codec::Name::EAC3), Codec::NameToString(streams[0]->GetCodec().GetName()));
    
//     RETURN_TEST("test_add_stream_with_codec", 0);
// }


// // Main function
// int main() {
//     int result = 0;
//     try {
//         result += test_file_initialization();
//         result += test_invalid_file_path();
//         result += test_matroska_compatible_codecs();
//         result += test_unsupported_codec();
//         result += test_codec_count_matroska();
//         result += test_container_identification();
//         result += test_pcm_wav_compatibility();
//         result += test_empty_file_exception();
//         result += test_avi_h264_codec();
//         result += test_add_stream_with_codec();
//     } catch (const StormByte::Multimedia::Exception& ex) {
//         std::cerr << ex.what() << std::endl;
//         result++;
//     }
//     if (result == 0) {
//         std::cout << "All tests passed!" << std::endl;
//     } else {
//         std::cout << result << " tests failed." << std::endl;
//     }
//     return result;
// }
int main() {
	return 0;
}