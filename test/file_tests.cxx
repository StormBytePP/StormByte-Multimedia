#include <multimedia/file.hxx>
#include <multimedia/container/matroska.hxx>
#include <multimedia/exception.hxx>
#include "test_handlers.h"

using namespace StormByte::Multimedia;

int test_mkv_file() {
	int result = 0;
	auto file = File(CurrentFileDirectory / "files" / "test.mkv");

	if (!std::dynamic_pointer_cast<Container::Matroska>(file.GetContainer())) {
		result = 1;
	}
	
	RETURN_TEST("test_mkv_file", result);
}

int main() {
    int result = 0;
    try {
		result += test_mkv_file();
    } catch (const StormByte::Multimedia::Exception& ex) {
        std::cerr << ex.what() << std::endl;
        result++;
    }
    if (result == 0) {
        std::cout << "All tests passed!" << std::endl;
    } else {
        std::cout << result << " tests failed." << std::endl;
    }
    return result;
}
