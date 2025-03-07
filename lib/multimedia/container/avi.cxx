#include <multimedia/container/avi.hxx>

using namespace StormByte::Multimedia::Container;

AVI::AVI():Container(Type::AVI, "avi") {}

bool AVI::IsStreamCompatible(const Stream::Stream&) {
	return true;
}