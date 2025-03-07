#include <multimedia/exception.hxx>

using namespace StormByte::Multimedia;

Exception::Exception(const std::string& message):StormByte::Exception(message) {}

Exception::Exception(std::string&& message) noexcept:StormByte::Exception(std::move(message)) {}

StreamNotCompatible::StreamNotCompatible(const Container::Type& container):
Exception("The stream is not compatible with container " + Container::TypeToString(container)) {}

CantAddStreams::CantAddStreams(const Container::Type& container):
Exception("Can't add streams to container " + Container::TypeToString(container)) {}