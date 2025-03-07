#include <multimedia/container/ogv.hxx>

using namespace StormByte::Multimedia::Container;

OGV::OGV():Container(Type::OGV, "ogv") {}

bool OGV::IsStreamCompatible(const Stream::Stream&) {
	return true;
}