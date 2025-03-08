#include <multimedia/exception.hxx>

using namespace StormByte::Multimedia;

Exception::Exception(const std::string& message):StormByte::Exception(message) {}

Exception::Exception(std::string&& message) noexcept:StormByte::Exception(std::move(message)) {}

CodecNotFound::CodecNotFound(const std::string& codecName):Exception("Codec not found: " + codecName) {}