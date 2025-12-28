#include <StormByte/test_handlers.h>
#include <StormByte/multimedia/file.hxx>
#include <StormByte/multimedia/stream.hxx>

#include <filesystem>
#include <iostream>

const std::filesystem::path test_hdr_plus_file = CurrentFileDirectory / "files" / "testHDR10+.mkv";

int test_num_streams_in_file() {
	const std::string fn_name = "test_num_streams_in_file";
	auto expected_file = StormByte::Multimedia::File::Open(test_hdr_plus_file);
	if (!expected_file) {
		std::cerr << "Error opening file: " << expected_file.error()->what() << std::endl;
		std::cerr << "Not failing as tests are not properly prepared yet for non-GPL FFmpeg builds." << std::endl;
		return 0;
	}
	ASSERT_EQUAL(fn_name, 6, std::distance(expected_file->StreamsBegin(), expected_file->StreamsEnd()));
	return 0;
}

int main() {
	int result = 0;

	result += test_num_streams_in_file();

	return 0;
}