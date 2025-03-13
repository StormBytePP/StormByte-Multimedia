#include <StormByte/multimedia/exception.hxx>

using namespace StormByte::Multimedia;

Exception::Exception(const std::string& message):StormByte::Exception(message) {}

Exception::Exception(std::string&& message) noexcept:StormByte::Exception(std::move(message)) {}

CodecNotFound::CodecNotFound(const std::string& codecName):Exception("Codec not found: " + codecName) {}

CodecNotSupported::CodecNotSupported(const Media::Container::Name& container, const Media::Codec::Name& codec):
Exception("Codec " + Media::Codec::Registry::Info(codec).s_name + " is not supported for container " + Media::Container::Registry::Info(container).s_name) {}

ContainerIsFull::ContainerIsFull(const Media::Container::Name& container):
Exception("Container " + Media::Container::Registry::Info(container).s_name + " is full") {}

ContainerNotFound::ContainerNotFound(const std::string& containerName):
Exception("Container " + containerName + " is not supported") {}