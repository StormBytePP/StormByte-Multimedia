#include <multimedia/container/ogg.hxx>

using namespace StormByte::Multimedia::Container;

OGG::OGG():Base(Type::OGG, "ogg") {}

bool OGG::IsStreamCompatible(const Stream::Stream&) {
	return true;
}