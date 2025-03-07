#include <multimedia/container/ogg.hxx>

using namespace StormByte::Multimedia::Container;

OGG::OGG():Container(Type::OGG, "ogg") {}

bool OGG::IsStreamCompatible(const Stream::Stream&) {
	return true;
}