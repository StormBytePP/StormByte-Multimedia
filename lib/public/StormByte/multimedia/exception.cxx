#include <StormByte/multimedia/exception.hxx>

#include <format>

using namespace StormByte::Multimedia;

Exception::Exception(const std::string& message):StormByte::Exception(message) {}

Exception::Exception(std::string&& message) noexcept:StormByte::Exception(std::move(message)) {}

CodecNotFound::CodecNotFound(const std::string& codecName):Exception(std::format("Codec not found: {}", codecName)) {}

CodecNotSupported::CodecNotSupported(const std::string& container, const std::string& codec):
Exception(std::format("Codec {} is not supported for container {}", codec, container)) {}

ContainerNotFound::ContainerNotFound(const std::string& containerName):
Exception(std::format("Container {} is not supported", containerName)) {}