#include <multimedia/container/oga.hxx>

using namespace StormByte::Multimedia::Container;

OGA::OGA():Container(Type::OGA, "oga") {}

bool OGA::IsStreamCompatible(const Stream::Stream&) {
	return true;
}