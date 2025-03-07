#include <multimedia/container/unknown.hxx>

using namespace StormByte::Multimedia::Container;

Unknown::Unknown():Container(Type::Unknown, "???") {}

bool Unknown::IsStreamCompatible(const Stream::Stream&) {
	return true;
}