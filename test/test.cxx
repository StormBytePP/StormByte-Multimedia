#include <StormByte/test_handlers.h>
#include <StormByte/multimedia/engine/demuxer.hxx>

#include <filesystem>
#include <iostream>

const std::filesystem::path test_hdr_plus_file = CurrentFileDirectory / "files" / "testHDR10+.mkv";

using namespace StormByte::Multimedia::Engine;

int test_num_streams_in_file() {
	const std::string fn_name = "test_num_streams_in_file";
	auto expected_file = Demuxer::Open(test_hdr_plus_file, Implementation::FFmpeg);
	if (!expected_file) {
		std::cerr << "Error opening file: " << expected_file.error()->what() << std::endl;
		// Even with GPL disabled the decoder should be available
		return 1;
	}
	ASSERT_EQUAL(fn_name, 6, std::distance(expected_file->Streams().begin(), expected_file->Streams().end()));
	return 0;
}

int main() {
	int result = 0;

	result += test_num_streams_in_file();

	return 0;
}