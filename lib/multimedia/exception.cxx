#include <multimedia/exception.hxx>

using namespace StormByte::Multimedia;

Exception::Exception(const std::string& message):StormByte::Exception(message) {}

Exception::Exception(std::string&& message) noexcept:StormByte::Exception(std::move(message)) {}

CodecNotFound::CodecNotFound(const std::string& codecName):Exception("Codec not found: " + codecName) {}

CodecNotSupported::CodecNotSupported(const Media::Container& container, const Media::Codec& codec):
Exception("Codec " + Media::CodecName(codec) + " is not supported for container " + Media::ContainerName(container)) {}

ContainerIsFull::ContainerIsFull(const Media::Container& container):
Exception("Container " + Media::ContainerName(container) + " is full") {}